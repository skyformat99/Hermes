#pragma once

#include <map>
#include <thread>
#include <atomic>
#include <chrono>
#include <string>
#include <memory>
#include <iostream>
#include <stdexcept>
#include <algorithm>
#include <functional>

#include <assert.h>
#undef NDEBUG

#include "google/protobuf/message.h"
#include "asio.hpp"

#define BUFFER_SIZE 2048
#define THREAD_POOL 100

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

/**
*
*
*
*
*
*
*
*
*/
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

  ~Session() noexcept { stop(); }

  deadline_timer& deadline() { return deadline_; }
  deadline_timer& heartbeat() { return heartbeat_; }

  void stop() {
    deadline_.cancel();
    heartbeat_.cancel();
    options_["state"] = false;
    options_["deadline"] = false;
    options_["heartbeat"] = false;
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

/**
*
*
*
*
*
*
*
*
*/
template <typename T>
class Stream : public std::enable_shared_from_this<Stream<T>> {
 public:
  typedef std::shared_ptr<Stream<T>> instance;

  static instance create(asio::io_context& io_service) {
    return instance(new Stream<T>(io_service));
  }

  T& socket() { return socket_; }
  Session<T>& session() { return session_; }

  void stop() {
    asio::error_code error;

    socket_.get_io_service().stop();
    session_.stop();
    socket_.shutdown(T::shutdown_both, error);
    if (error) throw asio::system_error(error);
    socket_.close();
  }

  std::tuple<asio::error_code, std::size_t> send(const std::string& message) {
    asio::error_code error;
    char buffer[BUFFER_SIZE] = {0};

    if (not message.size() or message.size() > BUFFER_SIZE)
      return std::make_tuple(asio::error::message_size, 0);

    std::strcpy(buffer, message.c_str());
    socket_.send(asio::buffer(buffer, message.size()), 0, error);

    if (error) return std::make_tuple(error, 0);

    return std::make_tuple(error, message.size());
  }

  std::tuple<asio::error_code, std::string> receive() {
    asio::error_code error;
    char buffer[BUFFER_SIZE] = {0};

    auto bytes = socket_.receive(asio::buffer(buffer, BUFFER_SIZE), 0, error);

    if (error) return std::make_tuple(error, std::string("!!Error!!"));

    if (not bytes)
      return std::make_tuple(error, std::string("ERR-0-BYTES-READ"));

    return std::make_tuple(error, std::string(buffer));
  }

  void async_send(const std::string& message, bool client,
                  const std::function<void(std::size_t)>& callback = nullptr) {
    if (not message.size())
      throw std::logic_error("[Messenger] attempt to send an empty message.");

    if (message.size() > BUFFER_SIZE)
      throw std::logic_error("[Messenger] Message size > " +
                             std::to_string(BUFFER_SIZE) +
                             ". Please modify define BUFFER_SIZE.");

    char buffer[BUFFER_SIZE] = {0};

    std::strcpy(buffer, message.c_str());
    socket_.async_send(asio::buffer(buffer, message.size()),
                       [&](const asio::error_code error, std::size_t bytes) {
      if (error) throw asio::system_error(error);

      if (not bytes) {
        throw std::runtime_error(
            "[Messenger] async_send failed. Unexpected error occurred.");
      }

      if (callback) callback(bytes);
    });

    if (client) socket_.get_io_service().run_one();
  }

  void async_receive(bool client,
                     const std::function<void(std::string)>& callback) {
    char buffer[BUFFER_SIZE] = {0};

    socket_.async_receive(
        asio::buffer(buffer, BUFFER_SIZE),
        [&](const asio::error_code& error, std::size_t bytes) {

          if (error) throw asio::system_error(error);

          if (not bytes or std::string(buffer).empty()) {
            throw std::runtime_error(
                "[Messenger] Unexpected error occurred. async_receive failed.");
          }

          callback(std::string(buffer));
        });
    if (client) socket_.get_io_service().run_one();
  }

 private:
  Stream(asio::io_context& io_service)
      : socket_(io_service), session_(socket_) {}

 private:
  T socket_;
  Session<T> session_;
};

/**
*
*
*
*
*
*
*
*
*/
template <typename T>
class Datagram : public std::enable_shared_from_this<Datagram<T>> {
 public:
  typedef std::shared_ptr<Datagram<T>> instance;

  static instance create(asio::io_context& io_service) {
    return instance(new Datagram<T>(io_service));
  }

  T& socket() { return socket_; }
  Session<T>& session() { return session_; }

 private:
  Datagram(asio::io_context& io_service)
      : socket_(io_service), session_(socket_) {}

 private:
  T socket_;
  Session<T> session_;
};

/**
*
*
*
*
*
*
*
*
*/
class Software {
 public:
  virtual ~Software() {}
  virtual void run() = 0;
  virtual void disconnect() = 0;
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


/**
*
*
*
*
*
*
*
*
*/class TCP_Client : public Software {
 public:
  TCP_Client(asio::io_context& io_service, const std::string& host,
         const std::string& port, bool async)
      : async_(async),
        host_(host),
        port_(port),
        connected_(false),
        service_(io_service),
        stream_(Stream<tcp::socket>::create(io_service)) {
    set_connection_handler(nullptr);
    set_disconnection_handler(nullptr);
  }

  TCP_Client(TCP_Client&&) = delete;
  TCP_Client(const TCP_Client&) = delete;
  TCP_Client& operator=(TCP_Client&&) = delete;
  TCP_Client& operator=(const TCP_Client&) = delete;

  ~TCP_Client() noexcept {
    if (connected_) disconnect();
  }

 private:
  bool async_;
  std::string host_;
  std::string port_;
  std::atomic<bool> connected_;
  asio::io_context::work service_;
  Stream<tcp::socket>::instance stream_;

 public:
  void run() {
    if (connected_)
      throw std::logic_error(
          "[Messenger] Error: Client already connected to: " + host_ + ":" +
          port_);

    tcp::endpoint endpoint(
      asio::ip::address::from_string(host_), std::stoi(port_)
    );

    if (not async_) {
      stream_->socket().connect(endpoint);
      connected_ = true;
      if (connect_handler_) connect_handler_();
      return;
    }

    stream_->socket().async_connect(endpoint,
                                    [this](const asio::error_code& error) {

      if (error)
        throw std::runtime_error(" [Messenger] Connection to host: " + host_ +
                                 " port: " + port_ + " failed.");
      connected_ = true;
      if (connect_handler_) connect_handler_();
    });
    service_.get_io_service().run_one();
    return;
  }

  void disconnect() {
    if (connected_) {
      stream_->stop();
      connected_ = false;
      if (disconnect_handler_) disconnect_handler_();
    }
  }

  std::size_t send(const std::string& message) {
    if (not connected_)
      throw std::logic_error(
          "[Messenger] Client is not connected. Call 'run' method once "
          "before.");

    auto result = stream_->send(message);

    if (std::get<0>(result)) {
      disconnect();
      throw asio::system_error(std::get<0>(result));
    }

    return std::get<1>(result);
  }

  std::string receive() {
    if (not connected_)
      throw std::logic_error(
          "[Messenger] Client is not connected. Call 'run' method once "
          "before.");

    auto received = stream_->receive();

    if (std::get<0>(received)) {
      disconnect();
      throw asio::system_error(std::get<0>(received));
    }

    if (std::get<1>(received) == "ERR-0-BYTES-READ")
      throw std::runtime_error("[Messenger] Receiving data from: " + host_ +
                               ":" + port_ + " failed. 0 bytes received.");

    return std::get<1>(received);
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

    try {
      stream_->async_send(message, true, callback);
    } catch (const std::exception& e) {
      disconnect();
      std::cerr << e.what() << std::endl;
    }
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

    try {
      stream_->async_receive(true, callback);
    } catch (const std::exception& e) {
      disconnect();
      std::cerr << e.what() << std::endl;
    }
  }
};

/**
*
*
*
*
*
*
*
*
*/
class TCP_Server : public Software {
 public:
  TCP_Server(const std::string& port, bool async)
      : async_(async),
        port_(port),
        acceptor_(io_service_),
        stream_(Stream<tcp::socket>::create(io_service_)) {
    set_connection_handler(nullptr);
    set_disconnection_handler(nullptr);
    async_ ? init_async_server() : init_sync_server();
  }

  TCP_Server(TCP_Server&&) = delete;
  TCP_Server(const TCP_Server&) = delete;
  TCP_Server& operator=(TCP_Server&&) = delete;
  TCP_Server& operator=(const TCP_Server&) = delete;

  ~TCP_Server() noexcept { disconnect(); }

 private:
  bool async_;
  std::string port_;
  asio::io_context io_service_;
  asio::ip::tcp::acceptor acceptor_;
  Stream<tcp::socket>::instance stream_;

 public:
  void run() {
    if (not async_) {
      asio::error_code error;

      acceptor_.accept(stream_->socket(), error);

      if (error) {
        disconnect();
        throw asio::system_error(error);
      }

      if (connect_handler_) connect_handler_();
      return;
    }

    std::vector<std::shared_ptr<std::thread>> threads;
    for (std::size_t i = 0; i < THREAD_POOL; ++i) {
      std::shared_ptr<std::thread> thread(
          new std::thread([&]() { io_service_.run(); }));
      threads.push_back(thread);
    }

    for (std::size_t i = 0; i < threads.size(); ++i) threads[i]->join();
  }

  void disconnect() {
    stream_->stop();
    acceptor_.close();
    if (disconnect_handler_) disconnect_handler_();
  }

  std::size_t send(const std::string& message) {
    auto result = stream_->send(message);

    if (std::get<0>(result)) {
      disconnect();
      throw asio::system_error(std::get<0>(result));
    }

    return std::get<1>(result);
  }

  std::string receive() {
    auto received = stream_->receive();

    if (std::get<0>(received)) {
      disconnect();
      throw asio::system_error(std::get<0>(received));
    }

    if (std::get<1>(received) == "ERR-0-BYTES-READ")
      throw std::runtime_error("[Messenger] Receiving data from port: "
                                + port_ + " failed. 0 bytes received.");

    return std::get<1>(received);
  }

  void async_send(const std::string& message,
                  const std::function<void(std::size_t)>& callback = nullptr) {
    if (not async_) {
      std::cerr << "[Messenger] Error: Synchronous server cannot perform";
      std::cerr << " asynchronous operations" << std::endl;
      return;
    }

    try {
      stream_->async_send(message, false, callback);
    } catch (const std::exception& e) {
      disconnect();
      std::cerr << e.what() << std::endl;
    }
  }

  void async_receive(const std::function<void(std::string)>& callback) {
    if (not async_) {
      std::cerr << "[Messenger] Error: Synchronous server cannot perform";
      std::cerr << " asynchronous operations" << std::endl;
      return;
    }

    try {
      stream_->async_receive(false, callback);
    } catch (const std::exception& e) {
      disconnect();
      std::cerr << e.what() << std::endl;
    }
  }

 private:
  void init_async_server() {
    tcp::endpoint endpoint(tcp::v4(), std::stoi(port_));
    acceptor_.open(endpoint.protocol());
    acceptor_.set_option(tcp::acceptor::reuse_address(true));
    acceptor_.bind(endpoint);
    acceptor_.listen();
    acceptor_.async_accept(
        stream_->socket(),
        [&](const asio::error_code& error) { handle_accept(error); });
  }

  void init_sync_server() {
    acceptor_ = tcp::acceptor(io_service_, tcp::endpoint(tcp::v4(), std::stoi(port_)));
    acceptor_.set_option(tcp::acceptor::reuse_address(true));
  }

  void handle_accept(const asio::error_code& error) {
    if (error) {
      disconnect();
      throw asio::system_error(error);
    }

    if (connect_handler_) connect_handler_();

    stream_.reset();
    stream_ = Stream<tcp::socket>::create(io_service_);
    acceptor_.async_accept(
        stream_->socket(),
        [&](const asio::error_code& error) { handle_accept(error); });
  }
};

/**
*
*
*
*
*
*
*
*
*/
class UDP_Client : public Software {
public:
  UDP_Client(asio::io_context& io_service, const std::string& host,
            const std::string &port, bool async)
  : async_(async),
    host_(host),
    port_(port),
    io_service_(io_service),
    datagram_(Datagram<udp::socket>::create(io_service)) {
      set_connection_handler(nullptr);
      set_disconnection_handler(nullptr);
  }

  UDP_Client(UDP_Client&&) = delete;
  UDP_Client(const UDP_Client&) = delete;
  UDP_Client& operator=(UDP_Client&&) = delete;
  UDP_Client& operator=(const UDP_Client&) = delete;

  ~UDP_Client() noexcept { disconnect(); }

  private:
   bool async_;
   std::string host_;
   std::string port_;
   udp::endpoint endpoint_;
   asio::io_context::work io_service_;
   Datagram<udp::socket>::instance datagram_;

 public:
   void run() {}

   void disconnect() {}

   std::size_t send(const std::string& message) { return 4; }

   std::string receive() {return std::string("");}

   void async_send(const std::string& message,
                   const std::function<void(std::size_t)>& callback = nullptr) {}

  void async_receive(const std::function<void(std::string)>& callback) {}

};

/**
*
*
*
*
*
*
*
*
*/
class UDP_Server : public Software {
public:
  UDP_Server(const std::string &port, bool async)
  : async_(async),
    port_(port),
    datagram_(Datagram<udp::socket>::create(io_service_)) {
      set_connection_handler(nullptr);
      set_disconnection_handler(nullptr);
  }

  UDP_Server(UDP_Server&&) = delete;
  UDP_Server(const UDP_Server&) = delete;
  UDP_Server& operator=(UDP_Server&&) = delete;
  UDP_Server& operator=(const UDP_Server&) = delete;

  ~UDP_Server() noexcept { disconnect(); }

  private:
   bool async_;
   std::string port_;
   udp::endpoint endpoint_;
   asio::io_context io_service_;
   Datagram<udp::socket>::instance datagram_;

 public:
   void run() {}

   void disconnect() {}

   std::size_t send(const std::string& message) { return 4; }

   std::string receive() {return std::string("");}

   void async_send(const std::string& message,
                   const std::function<void(std::size_t)>& callback = nullptr) {}

  void async_receive(const std::function<void(std::string)>& callback) {}

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
*   design:
*     https://github.com/TommyStarK/Hermes/blob/master/DESIGN.md
*   exemples:
*     https://github.com/TommyStarK/Hermes/blob/master/tests/messenger.cpp
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

  Messenger(Messenger&&) = delete;
  Messenger(const Messenger&) = delete;
  Messenger& operator=(Messenger&&) = delete;
  Messenger& operator=(const Messenger&) = delete;
  ~Messenger() noexcept {}

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
  //
  // NOTE: 5th bith will be set to handle type of ip (ipv4/ipv6)
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
        messenger_ =
            std::make_shared<TCP_Client>(io_service_, host, port, async_);
        break;

      case UDP_CLIENT:
        messenger_ =
            std::make_shared<UDP_Client>(io_service_, host, port, async_);
        break;

      case TCP_SERVER:
        messenger_ = std::make_shared<TCP_Server>(port, async_);
        break;

      case UDP_SERVER:
        messenger_ = std::make_shared<UDP_Server>(port, async_);
        break;

      default:
        throw std::runtime_error(
          "[Messenger] initialize_software: Unexpected occurred.");
        break;
    }
  }

 private:
  bool async_;
  unsigned char options_;
  asio::io_context io_service_;
  std::shared_ptr<Software> messenger_;

 public:
  void run() { messenger_->run(); }

  void disconnect() { messenger_->disconnect(); }

  void async_receive(const std::function<void(std::string)>& callback) {
    messenger_->async_receive(callback);
  }

  void async_send(const std::string& msg,
                  const std::function<void(std::size_t)>& callback = nullptr) {
    messenger_->async_send(msg, callback);
  }

  void set_connection_handler(const std::function<void()>& callback) {
    messenger_->set_connection_handler(callback);
  }

  void set_disconnection_handler(const std::function<void()>& callback) {
    messenger_->set_disconnection_handler(callback);
  }

  std::string receive() { return messenger_->receive(); }
  std::size_t send(const std::string& msg) { return messenger_->send(msg); }
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
*  unknown behavior will happen.
*
* @protocol:
*   TCP
*
* @see:
*   design:
*       https://github.com/TommyStarK/Hermes/blob/master/DESIGN.md
*   exemples:
*       https://github.com/TommyStarK/Hermes/blob/master/tests/protobuff.cpp
*/
namespace protobuf {

using namespace google::protobuf;

// Synchronous writting operation.
template <typename T>
std::size_t send(const std::string& host, const std::string& port,
                 const T& message) {
  assert(message.GetDescriptor() and std::stoi(port) >= 0);

  std::string serialized;
  char buffer[BUFFER_SIZE] = {0};
  asio::io_context io_service;
  asio::error_code error = asio::error::host_not_found;

  tcp::resolver resolver(io_service);
  tcp::socket socket(io_service);
  auto endpoint = resolver.resolve(tcp::resolver::query(host, port));

  message.SerializeToString(&serialized);
  std::strcpy(buffer, serialized.c_str());
  asio::connect(socket, endpoint, error);

  if (error) {
    socket.shutdown(tcp::socket::shutdown_both);
    socket.close();
    throw std::runtime_error("[protobuf] Connection to host: " + host +
                             " port: " + port + " failed.");
  }

  asio::write(socket, asio::buffer(buffer, serialized.size()), error);

  if (error) {
    socket.shutdown(tcp::socket::shutdown_both);
    socket.close();
    throw asio::system_error(error);
  }

  socket.shutdown(tcp::socket::shutdown_both);
  socket.close();
  return serialized.size();
}

// Synchronous reading operation.
template <typename T>
T receive(const std::string& port) {
  assert(std::stoi(port) >= 0);
  asio::error_code error;
  char buffer[BUFFER_SIZE] = {0};
  asio::io_context io_service;

  tcp::socket socket(io_service);
  tcp::acceptor acceptor(io_service, tcp::endpoint(tcp::v4(), std::stoi(port)));
  acceptor.set_option(tcp::acceptor::reuse_address(true));
  acceptor.accept(socket, error);

  if (error) {
    socket.shutdown(tcp::socket::shutdown_both);
    socket.close();
    acceptor.close();
    throw std::runtime_error("[protobuf] Accepting connection on port: " +
                             port + " failed.");
  }

  socket.read_some(asio::buffer(buffer), error);

  if (error == asio::error::eof) {
    acceptor.close();
    throw std::runtime_error("[protobuf] connection closed.");
  } else if (error) {
    socket.shutdown(tcp::socket::shutdown_both);
    socket.close();
    acceptor.close();
    throw asio::system_error(error);
  }

  T result;
  result.ParseFromString(std::string(buffer));
  socket.shutdown(tcp::socket::shutdown_both);
  socket.close();
  acceptor.close();
  return result;
}

template <typename T>
void async_send(const std::string& host, const std::string& port,
                const T& message,
                const std::function<void(std::size_t)>& callback = nullptr) {
  assert(message.GetDescriptor() and std::stoi(port) >= 0);

  char buffer[BUFFER_SIZE] = {0};
  std::string serialized;
  asio::io_context io_service;

  tcp::socket socket(io_service);
  tcp::resolver resolver(io_service);
  message.SerializeToString(&serialized);
  auto endpoint = resolver.resolve(tcp::resolver::query(host, port));
  socket.async_connect(endpoint->endpoint(),
                       [&](const asio::error_code& error) {

    if (error) {
      socket.shutdown(tcp::socket::shutdown_both);
      socket.close();
      throw std::runtime_error("[protobuf] Connection to host: " + host +
                               " port: " + port + " failed.");
    }

    std::strcpy(buffer, serialized.c_str());
    asio::async_write(socket, asio::buffer(buffer, serialized.size()),
                      [&](const asio::error_code& error, std::size_t bytes) {

      if (error) {
        socket.shutdown(tcp::socket::shutdown_both);
        socket.close();
        throw std::system_error(error);
      }

      if (not bytes) {
        socket.shutdown(tcp::socket::shutdown_both);
        socket.close();
        throw std::runtime_error(
            "[protobuf] Unexpected error occurred: 0 bytes sent");
      }

      if (callback) callback(bytes);
      socket.shutdown(tcp::socket::shutdown_both);
      socket.close();
    });

  });
  std::thread([&]() { io_service.run(); }).join();
}

// Asynchronous reading operation.
template <typename T>
void async_receive(const std::string& port,
                   const std::function<void(T)>& callback) {
  assert(std::stoi(port) >= 0);
  asio::error_code error;
  char buffer[BUFFER_SIZE] = {0};
  asio::io_context io_service;

  tcp::socket socket(io_service);
  tcp::acceptor acceptor(io_service);
  tcp::endpoint endpoint(tcp::v4(), std::stoi(port));

  acceptor.open(endpoint.protocol());
  acceptor.set_option(tcp::acceptor::reuse_address(true));
  acceptor.bind(endpoint);
  acceptor.listen();
  acceptor.async_accept(socket, [&](const asio::error_code& error) {

    if (error) {
      socket.shutdown(tcp::socket::shutdown_both);
      socket.close();
      acceptor.close();
      throw std::runtime_error("[protobuf] Accept connection on port: " + port +
                               " failed.");
    }

    asio::async_read(socket, asio::buffer(buffer, BUFFER_SIZE),
                     [&](const asio::error_code& error, std::size_t bytes) {

      if (error and error != asio::error::eof) {
        socket.shutdown(tcp::socket::shutdown_both);
        socket.close();
        acceptor.close();
        throw asio::system_error(error);
      }

      if (not bytes or std::string(buffer).empty()) {
        acceptor.close();
        socket.shutdown(tcp::socket::shutdown_both);
        socket.close();
        throw std::runtime_error(
            "[protobuf] Unexpected error occurred: 0 bytes received");
      }

      T response;
      response.ParseFromString(std::string(buffer));
      callback(response);
      acceptor.close();
      socket.shutdown(tcp::socket::shutdown_both);
      socket.close();
    });
  });
  std::thread([&]() { io_service.run(); }).join();
}

}  // namespace protobuf
}  // namespace Hermes
