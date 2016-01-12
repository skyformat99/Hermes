#pragma once

#include <map>
#include <ctime>
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
* Ability namespace.
*
* @description:
*   Various handlers.
*
*/
namespace Ability {
// get nth bit of an unsigned char.
inline char get_n_bit(unsigned char* c, int n) {
  return (*c & (1 << n)) ? 1 : 0;
}

// change nth bit of an unsigned char to the given value.
inline void change_n_bit(unsigned char* c, int n, int value) {
  *c = (*c | (1 << n)) & ((value << n) | ((~0) ^ (1 << n)));
}

template <class A, class... B>
void do_after(unsigned int seconds, bool async, A&& a, B&&... b) {
  std::function<typename std::result_of<A(B...)>::type()> task(
    std::bind(std::forward<A>(a), std::forward<B>(b)...)
  );
  if (async) {
    std::thread([seconds, task](){
      std::this_thread::sleep_for(std::chrono::seconds(seconds));
      task();
    }).detach();
  }
  else {
    std::this_thread::sleep_for(std::chrono::seconds(seconds));
    task();
  }
}
}  // namespace Ability

enum socket_state { UNUSED = 0, READING, WRITTING };

typedef std::function<void(std::size_t)> callback;
typedef std::function<void(const std::string&)> rcallback;
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

 private:
  Stream(asio::io_context& io_service)
      : socket_(io_service), session_(socket_) {}

 private:
  T socket_;
  Session<T> session_;
};

class Software {
 public:
  virtual void run() = 0;
  virtual void disconnect() = 0;
  virtual std::string receive() = 0;
  virtual std::size_t send(const std::string&);
  virtual void async_send(const std::string&,
                          const callback& handler = nullptr) = 0;
  virtual void async_receive(const rcallback& handler = nullptr) = 0;
};

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
  int handle_options() {
    if (Ability::get_n_bit(&options_, 0) and Ability::get_n_bit(&options_, 2))
      return 1;

    if (Ability::get_n_bit(&options_, 0) and Ability::get_n_bit(&options_, 3))
      return 2;

    if (Ability::get_n_bit(&options_, 1) and Ability::get_n_bit(&options_, 2))
      return 3;

    if (Ability::get_n_bit(&options_, 1) and Ability::get_n_bit(&options_, 3))
      return 4;

    return 0;
  }

  void resolve_software(const std::string& software,
                        const std::string& protocol) {
    auto s = software;
    auto p = protocol;

    std::transform(s.begin(), s.end(), s.begin(), ::tolower);
    std::transform(p.begin(), p.end(), p.begin(), ::tolower);

    if (s == "client") Ability::change_n_bit(&options_, 0, 1);

    if (s == "server") Ability::change_n_bit(&options_, 1, 1);

    if (p == "tcp") Ability::change_n_bit(&options_, 2, 1);

    if (p == "udp") Ability::change_n_bit(&options_, 3, 1);

    if (async_ == true) Ability::change_n_bit(&options_, 4, 1);

    if (not options_)
      throw std::invalid_argument(
          "[Messenger] software has to be a client or a server and "
          "protocol tcp or udp.");
  }

  void initialize_software(const std::string& host, const std::string& port) {
    int flag = handle_options();

    switch (flag) {
      default:
        break;
    }
  }

 private:
  bool async_;
  unsigned char options_;
  static asio::io_context io_service_;
  std::shared_ptr<Software> messenger_;

 public:
  void run() { messenger_->run(); }
  void disconnect() { messenger_->disconnect(); }
  Software* get_messenger() { return messenger_.get(); }
  std::string receive() { return messenger_->receive(); }
  std::size_t send(const std::string& msg) { return messenger_->send(msg); }

  void async_send(const std::string& msg, const callback& handler = nullptr) {
    messenger_->async_send(msg, handler);
  }

  void async_receive(const rcallback& handler = nullptr) {
    messenger_->async_receive(handler);
  }
};


/**
* Module: serialization - namespace protobuf
*
* @description:
*   Contains Hermes protobuf operations.
*   Allows user to send a serialized version of their protobuf message.
*
* @required:
*   Define .proto model
*   Generate according classes with protoc
*
* @protocol:
*   TCP
*/
namespace protobuf {

using namespace google::protobuf;

// Synchronous writting operation.
template <typename T>
std::size_t send(const std::string& host, const std::string& port,
                 std::shared_ptr<T> message) {
  assert(message->GetDescriptor());

  std::string serialized;
  char buffer[2048] = {0};
  asio::io_context io_service;
  asio::error_code error = asio::error::host_not_found;

  tcp::resolver resolver(io_service);
  tcp::socket socket(io_service);
  auto endpoint = resolver.resolve(tcp::resolver::query(host, port));

  message->SerializeToString(&serialized);
  std::strcpy(buffer, serialized.c_str());
  asio::connect(socket, endpoint, error);

  if (error) {
    socket.close();
    throw asio::system_error(error);
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
std::shared_ptr<T> receive(const std::string& port) {
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
    throw std::runtime_error(
        "[protobuf] Unexpected error occurred: acceptor failed to accept "
        "socket.");
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

  auto result = std::make_shared<T>();
  result->ParseFromString(std::string(buffer));
  return result;
}

// Asynchronous writting operation.
template <typename T>
void async_send(const std::string& host, const std::string& port,
                std::shared_ptr<T> message, const callback& handler = nullptr) {
  assert(message->GetDescriptor());

  char buffer[2048] = {0};
  std::string serialized;
  asio::io_context io_service;

  tcp::socket socket(io_service);
  tcp::resolver resolver(io_service);
  message->SerializeToString(&serialized);
  auto endpoint = resolver.resolve(tcp::resolver::query(host, port));
  socket.async_connect(endpoint->endpoint(),
                       [&](const asio::error_code& error) {

    if (error) {
      socket.close();
      throw std::system_error(error);
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

      if (handler) handler(bytes);

    });

  });
  io_service.run();
}

// Asynchronous reading operation.
template <typename T>
void async_receive(const std::string& port, T* const message,
                   const rcallback& handler = nullptr) {
  assert(message->GetDescriptor());

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
      throw asio::system_error(error);
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

      if (handler)
        handler(std::string(buffer));
      else
        message->ParseFromString(std::string(buffer));

    });

  });
  io_service.run();
}

}  // namespace protobuf
}  // namespace Hermes
