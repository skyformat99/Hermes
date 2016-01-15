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

/**
* Messenger class allows you to create network software.
*
* @param:
*     - software (client/server)
*     - protocol (tcp/udp)
*     - async
*     - port
*     - host (set by default to localhost)
*
* @see:
*   design: https://github.com/TommyStarK/Hermes/blob/master/DESIGN.md
*   exemples:
*https://github.com/TommyStarK/Hermes/blob/master/tests/messenger.cpp
*/
class Messenger {
 public:
  Messenger(const std::string& software, const std::string& protocol,
            bool async, const std::string& port,
            const std::string& host = "127.0.0.1")
      : async_(async), options_(0) {
    resolve_software(software, protocol);
    initialize_software(host, port);
  }

  Messenger(const Messenger&) = delete;
  Messenger& operator=(const Messenger&) = delete;
  ~Messenger() {}

 private:
  // get nth bit of an unsigned char.
  inline char get_n_bit(unsigned char* c, int n) {
    return (*c & (1 << n)) ? 1 : 0;
  }

  // change nth bit of an unsigned char to the given value.
  inline void change_n_bit(unsigned char* c, int n, int value) {
    *c = (*c | (1 << n)) & ((value << n) | ((~0) ^ (1 << n)));
  }

  // handle options set from parsing parameters.
  int handle_options() {
    if (get_n_bit(&options_, 0) and get_n_bit(&options_, 2)) return TCP_CLIENT;

    if (get_n_bit(&options_, 0) and get_n_bit(&options_, 3)) return UDP_CLIENT;

    if (get_n_bit(&options_, 1) and get_n_bit(&options_, 2)) return TCP_SERVER;

    if (get_n_bit(&options_, 1) and get_n_bit(&options_, 3)) return UDP_SERVER;

    return NONE;
  }

  // resolve software by parsing parameters and settings according bit
  // of options_ to 1.
  //
  // @param:
  //    software, protocol
  //
  // @design:
  //    bit field: 76543210
  //        * 0 = client   2 = tcp    4 = async
  //        * 1 = server   3 = udp    5-7 = unused (future feature).
  //    If according bit is set to 1, specification is required.
  void resolve_software(const std::string& software,
                        const std::string& protocol) {
    auto s = software;
    auto p = protocol;

    std::transform(s.begin(), s.end(), s.begin(), ::tolower);
    std::transform(p.begin(), p.end(), p.begin(), ::tolower);

    if (s == "client") change_n_bit(&options_, 0, 1);

    if (s == "server") change_n_bit(&options_, 1, 1);

    if (p == "tcp") change_n_bit(&options_, 2, 1);

    if (p == "udp") change_n_bit(&options_, 3, 1);

    if (async_ == true) change_n_bit(&options_, 4, 1);

    if (not options_)
      throw std::invalid_argument(
          "[Messenger] software has to be a client or a server and "
          "protocol tcp or udp.");
  }

  // Init required network software
  void initialize_software(const std::string& host, const std::string& port) {
    int flag = handle_options();

    switch (flag) {
      case TCP_CLIENT:
        messenger_ = std::make_shared<Client<asio::ip::tcp>>(io_service_, host,
                                                             port, async_);
        break;

      case UDP_CLIENT:
        messenger_ = std::make_shared<Client<asio::ip::udp>>(io_service_, host,
                                                             port, async_);
        break;

      case TCP_SERVER:
        messenger_ =
            std::make_shared<Server<asio::ip::tcp>>(io_service_, port, async_);
        break;

      case UDP_SERVER:
        messenger_ =
            std::make_shared<Server<asio::ip::udp>>(io_service_, port, async_);
        break;

      default:
        break;
    }
  }

 private:
  bool async_;
  unsigned char options_;
  asio::io_context io_service_;
  std::shared_ptr<Software> messenger_;

 public:
  void run(const std::function<void()>& callback = nullptr) {
    messenger_->run(callback);
  }

  void disconnect(const std::function<void()>& callback = nullptr) {
    messenger_->disconnect(callback);
  }

  void async_receive(const std::function<void(std::string)>& callback) {
    messenger_->async_receive(callback);
  }

  void async_send(const std::string& msg,
                  const std::function<void(std::size_t)>& callback = nullptr) {
    messenger_->async_send(msg, callback);
  }

  Software* get_messenger() { return messenger_.get(); }
  std::string receive() { return messenger_->receive(); }
  std::size_t send(const std::string& msg) { return messenger_->send(msg); }

  void set_connection_handler(const std::function<void()>& callback) {
    messenger_->set_connection_handler(callback);
  }

  void set_disconnection_handler(const std::function<void()>& callback) {
    messenger_->set_disconnection_handler(callback);
  }
};

/**
* Module: serialization - namespace protobuf
*
* @description:
*   Contains Hermes protobuf operations.
*   Allows user to send/receive a serialized version of their protobuf
*   messages.
*
* @required:
*   Define Google .proto model.
*   Generate according classes with protoc binary.
*
*
* @threadsafe:
*  All asio's network variables belong to their scope function so, many threads
*  could do multiple calls in the same time to Hermes protobuf operations. No
*unknown
*  behavior will happen.
*
* @protocol:
*   TCP
*
* @see:
*   design: https://github.com/TommyStarK/Hermes/blob/master/DESIGN.md
*   exemples:
*https://github.com/TommyStarK/Hermes/blob/master/tests/protobuff.cpp
*/
namespace protobuf {

using namespace google::protobuf;

// Synchronous writting operation.
template <typename T>
std::size_t send(const std::string& host, const std::string& port,
                 const T& message) {
  assert(message.GetDescriptor());

  std::string serialized;
  char buffer[2048] = {0};
  asio::io_context io_service;
  asio::error_code error = asio::error::host_not_found;

  tcp::resolver resolver(io_service);
  tcp::socket socket(io_service);
  auto endpoint = resolver.resolve(tcp::resolver::query(host, port));

  message.SerializeToString(&serialized);
  std::strcpy(buffer, serialized.c_str());
  asio::connect(socket, endpoint, error);

  if (error) {
    socket.close();
    throw std::runtime_error("[protobuf] Connection to host: " + host +
                             " port: " + port + " failed.");
  }

  asio::write(socket, asio::buffer(buffer, serialized.size()), error);

  if (error) {
    socket.close();
    throw asio::system_error(error);
  }
  return serialized.size();
}

// Synchronous reading operation.
template <typename T>
T receive(const std::string& port) {
  assert(std::stoi(port));
  asio::error_code error;
  char buffer[2048] = {0};
  asio::io_context io_service;

  tcp::socket socket(io_service);
  tcp::acceptor acceptor(io_service, tcp::endpoint(tcp::v4(), std::stoi(port)));
  tcp::acceptor::reuse_address option(true);
  acceptor.set_option(option);
  acceptor.accept(socket, error);

  if (error) {
    acceptor.close();
    socket.close();
    throw std::runtime_error("[protobuf] Accepting connection on port: " +
                             port + " failed.");
  }

  socket.read_some(asio::buffer(buffer), error);

  if (error == asio::error::eof) {
    acceptor.close();
    throw std::runtime_error("[protobuf] connection closed.");
  } else if (error) {
    acceptor.close();
    socket.close();
    throw asio::system_error(error);
  }

  T result;
  result.ParseFromString(std::string(buffer));
  return result;
}

// Asynchronous writting operation.
template <typename T>
void async_send(const std::string& host, const std::string& port,
                const T& message,
                const std::function<void(std::size_t)>& callback = nullptr) {
  assert(message.GetDescriptor());

  char buffer[2048] = {0};
  std::string serialized;
  asio::io_context io_service;

  tcp::socket socket(io_service);
  tcp::resolver resolver(io_service);
  message.SerializeToString(&serialized);
  auto endpoint = resolver.resolve(tcp::resolver::query(host, port));
  socket.async_connect(endpoint->endpoint(),
                       [&](const asio::error_code& error) {

    if (error) {
      socket.close();
      throw std::runtime_error("[protobuf] Connection to host: " + host +
                               " port: " + port + " failed.");
    }

    std::strcpy(buffer, serialized.c_str());
    asio::async_write(socket, asio::buffer(buffer, serialized.size()),
                      [&](const asio::error_code& error, std::size_t bytes) {

      if (error) {
        socket.close();
        throw std::system_error(error);
      }

      if (not bytes)
        throw std::runtime_error(
            "[protobuf] Unexpected error occurred: 0 bytes sent");

      if (callback) callback(bytes);
    });

  });
}

// Asynchronous reading operation.
template <typename T>
void async_receive(const std::string& port,
                   const std::function<void(T)>& callback) {
  assert(callback);
  char buffer[2048] = {0};
  asio::io_context io_service;

  tcp::socket socket(io_service);
  tcp::acceptor acceptor(io_service, tcp::endpoint(tcp::v4(), std::stoi(port)));
  tcp::acceptor::reuse_address option(true);
  acceptor.set_option(option);
  acceptor.async_accept(socket, [&](const asio::error_code& error) {

    if (error) {
      acceptor.close();
      socket.close();
      throw std::runtime_error("[protobuf] Accepting connection on port: " +
                               port + " failed.");
    }

    asio::async_read(socket, asio::buffer(buffer, 2048),
                     [&](const asio::error_code& error, std::size_t bytes) {

      if (error and error != asio::error::eof) {
        acceptor.close();
        socket.close();
        throw asio::system_error(error);
      }

      if (not bytes or std::string(buffer).empty())
        throw std::runtime_error(
            "[protobuf] Unexpected error occurred: 0 bytes received");

      T response;
      response.ParseFromString(std::string(buffer));
      callback(response);
    });

  });
}

}  // namespace protobuf
}  // namespace Hermes
