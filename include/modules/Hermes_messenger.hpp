#pragma once

#include <map>
#include <ctime>
#include <atomic>
#include <chrono>
#include <string>
#include <thread>
#include <memory>
#include <iostream>
#include <stdexcept>
#include <algorithm>
#include <functional>

#include <assert.h>

#include "google/protobuf/message.h"
#include "asio.hpp"

namespace Hermes {

using namespace asio::ip;

/**
*   Module - Messenger
*
*   @description:
*     A global entity which allows you to create network softwares. A messenger
*     could be a client or a server and it handles natively TCP/UDP protocol
*     as well as asynchronous operations.
*
*  @required:
*   - asio 1.10.6.
*
*
*
*/
enum socket_state { UNUSED = 0, READING, WRITTING };
enum software_type { NONE = 0, TCP_CLIENT, UDP_CLIENT, TCP_SERVER, UDP_SERVER };
typedef asio::basic_waitable_timer<std::chrono::system_clock> deadline_timer;

template <class T>
class Session {
 public:
  Session(T& socket)
      : socket_(socket),
        state_(UNUSED),
        deadline_(socket.get_io_service()),
        heartbeat_(socket.get_io_service()),
        heartbeat_message_("<3") {
    options_["state"] = false;
    options_["deadline"] = false;
    options_["heartbeat"] = false;
  }

  ~Session() { stop(); }

  deadline_timer& deadline() { return deadline_; }
  deadline_timer& heartbeat() { return heartbeat_; }

  void stop() {
    options_["state"] = false;
    options_["deadline"] = false;
    options_["heartbeat"] = false;
    deadline_.cancel();
    heartbeat_.cancel();
    socket_.close();
  }

  void activate_option(const std::string& name) {
    if (options_.find(name) == options_.end())
      throw std::invalid_argument("[Session] Error: invalid option.");
    options_[name] = true;
  }

  void set_state_to_socket(socket_state state) { state_ = state; }
  void set_heartbeat_message(const std::string& s) { heartbeat_message_ = s; }

  bool is_socket_unused() const { return state_ == UNUSED; }
  bool is_ready_for_reading() const { return state_ == READING; }
  bool is_ready_for_writting() const { return state_ == WRITTING; }

  bool is_option_activated(const std::string& name) {
    return options_.find(name) != options_.end() ? options_[name] : false;
  }

  std::string get_heartbeat_message() const { return heartbeat_message_; }

 private:
  T& socket_;
  socket_state state_;
  deadline_timer deadline_;
  deadline_timer heartbeat_;
  std::string heartbeat_message_;
  std::map<std::string, bool> options_;
};

template <typename T>
class Stream : public std::enable_shared_from_this<Stream<T>> {
 public:
  typedef std::shared_ptr<Stream<T>> instance;

  static instance create(asio::io_context& io_service) {
    return instance(new Stream<T>(io_service));
  }

  T& socket() { return socket_; }
  Session<T>& session() { return session_; }

  void close() {
    socket_.close();
    session_.stop();
  }

 private:
  Stream(asio::io_context& io_service)
      : socket_(io_service), session_(socket_) {}

 private:
  T socket_;
  Session<T> session_;
};

class Software {
 public:
  virtual ~Software() {}
  virtual void run(const std::function<void()>& callback = nullptr) = 0;
  virtual void disconnect(const std::function<void()>& callback = nullptr) = 0;
  virtual std::string receive() = 0;
  virtual std::size_t send(const std::string&) = 0;
  virtual void async_receive(const std::function<void(std::string)>&) = 0;
  virtual void async_send(
      const std::string&,
      const std::function<void(std::size_t)>& callback = nullptr) = 0;

  void set_connection_handler(const std::function<void()>& callback) {
    connect_handler_ = callback;
  }

  void set_disconnection_handler(const std::function<void()>& callback) {
    disconnect_handler_ = callback;
  }

 protected:
  std::function<void()> connect_handler_;
  std::function<void()> disconnect_handler_;
};

template <typename T>
class Client : public Software {
 public:
  Client(asio::io_context& io_service, const std::string& host,
         const std::string& port, bool async)
      : async_(async),
        host_(host),
        port_(port),
        connected_(false),
        io_service_(io_service),
        stream_(Stream<typename T::socket>::create(io_service)) {
    set_connection_handler(nullptr);
    set_disconnection_handler(nullptr);
  }

  Client(const Client&) = delete;
  Client& operator=(const Client&) = delete;

  ~Client() {
    if (connected_) disconnect();
  }

 private:
  bool async_;
  std::string host_;
  std::string port_;
  std::atomic<bool> connected_;
  asio::io_context& io_service_;
  typename Stream<typename T::socket>::instance stream_;

 public:
  void run(const std::function<void()>& callback = nullptr) {
    if (connected_)
      throw std::logic_error(
          "[Messenger] Error: Client already connected to: " + host_ + ":" +
          port_);

    char buffer[2048] = {0};
    asio::error_code error = asio::error::host_not_found;

    asio::ip::basic_endpoint<T> endpoint(asio::ip::address::from_string(host_),
                                         std::stoi(port_));

    if (not async_) {
      stream_->socket().connect(endpoint);
      connected_ = true;
      return;
    }

    stream_->socket().async_connect(
        endpoint, [this, &callback](const asio::error_code& error) {

          if (error) {
            stream_->close();
            throw std::runtime_error(" [Messenger] Connection to host: " +
                                     host_ + " port: " + port_ + " failed.");
          }

          connected_ = true;
          if (callback)
            callback();
          else if (connect_handler_)
            connect_handler_();
        });

    io_service_.run();
    return;
  }

  void disconnect(const std::function<void()>& callback = nullptr) {
    if (connected_) {
      connected_ = false;

      if (callback)
        callback();
      else if (disconnect_handler_)
        disconnect_handler_();
      else
        stream_->close();
    }
  }

  std::size_t send(const std::string& message) {
    if (not connected_)
      throw std::logic_error(
          "[Messenger] Client is not connected. Call 'run' method once "
          "before.");

    asio::error_code error;
    char buffer[2048] = {0};

    std::strcpy(buffer, message.c_str());
    stream_->socket().send(asio::buffer(buffer, message.size()), 0, error);

    if (error) {
      stream_->close();
      throw std::runtime_error(
          "[Messenger] Sending message through socket bind on: " + host_ + ":" +
          port_ + " failed.");
    }

    return message.size();
  }

  std::string receive() {
    if (not connected_)
      throw std::logic_error(
          "[Messenger] Client is not connected. Call 'run' method once "
          "before.");

    std::string response;
    asio::error_code error;
    char buffer[2048] = {0};

    auto bytes =
        stream_->socket().receive(asio::buffer(buffer, 2048), 0, error);

    if (response = buffer, error or bytes != response.size()) {
      stream_->close();
      throw std::runtime_error("[Messenger] Receiving data from: " + host_ +
                               ":" + port_ + " failed.");
    }
    return response;
  }

  void async_send(const std::string& message,
                  const std::function<void(std::size_t)>& callback = nullptr) {
    if (not connected_)
      throw std::logic_error(
          "[Messenger] Client is not connected. Call 'run' method once "
          "before.");

    if (not async_) {
      std::cerr << "[Messenger] Error: Synchronous Client cannot perform";
      std::cerr << " asynchronous operations" << std::endl;
      return;
    }

    asio::error_code error;
    char buffer[2048] = {0};

    std::strcpy(buffer, message.c_str());
    stream_->socket().async_send(asio::buffer(buffer, message.size()),
                                 [&](const asio::error_code& error,
                                     std::size_t bytes) {

      if (error) {
        stream_->close();
        throw asio::system_error(error);
      }

      if (not bytes or bytes != message.size()) {
        stream_->close();
        throw std::runtime_error(
            "[Messenger] async_send failed. Unexpected error 0 bytes sent.");
      }

      if (callback) callback(bytes);
    });
  }

  void async_receive(const std::function<void(std::string)>& callback) {
    if (not connected_)
      throw std::logic_error(
          "[Messenger] Client is not connected. Call 'run' method once "
          "before.");

    if (not async_) {
      std::cerr << "[Messenger] Error: Synchronous Client cannot perform";
      std::cerr << " asynchronous operations" << std::endl;
      return;
    }

    asio::error_code error;
    char buffer[2048] = {0};

    stream_->socket().async_receive(
        asio::buffer(buffer, 2048),
        [&](const asio::error_code& error, std::size_t bytes) {
          if (error and error != asio::error::eof) {
            stream_->close();
            throw asio::system_error(error);
          }

          if (not bytes or std::string(buffer).empty()) {
            stream_->close();
            throw std::runtime_error(
                "[Messenger] Unexpected error occurred. async_receive failed.");
          }

          callback(std::string(buffer));
        });
  }
};

template <typename T>
class Server : public Software {
 public:
  Server(asio::io_context& io_service, const std::string& port, bool async)
      : async_(async),
        port_(port),
        connected_(false),
        io_service_(io_service),
        stream_(Stream<typename T::socket>::create(io_service)) {
    set_connection_handler(nullptr);
    set_disconnection_handler(nullptr);
  }

  Server(const Server&) = delete;
  Server& operator=(const Server&) = delete;

  ~Server() {
    if (connected_) disconnect();
  }

 private:
  bool async_;
  std::string port_;
  std::atomic<bool> connected_;
  asio::io_context& io_service_;
  typename Stream<typename T::socket>::instance stream_;

 public:
  void run(const std::function<void()>& callback = nullptr) {}
  void disconnect(const std::function<void()>& callback = nullptr) {}
  std::string receive() { return ""; }
  std::size_t send(const std::string& message) { return 4; }
  void async_receive(const std::function<void(std::string)>& callback) {}
  void async_send(const std::string& message,
                  const std::function<void(std::size_t)>& callback = nullptr) {}
};
}  // namespace Hermes
