#pragma once

#include <mutex>
#include <atomic>
#include <chrono>
#include <memory>
#include <string>
#include <thread>
#include <vector>
#include <iostream>
#include <stdexcept>
#include <functional>
#include <condition_variable>

#include "asio.hpp"
#include "google/protobuf/message.h"

namespace hermes {


/**
*   @brief: Hermes core functionalities
*
*   @description: core namespace contains various classes to provide basic
*   features such as I/O services or errors handling.
*
*/
namespace core {

// Size used for buffers.
// Modify the value if you need a bigger size.
static unsigned int const BUFFER_SIZE = 2048;


/**
*  @brief: Your program's link to your operating system I/O services.
*
*  @description: Asio's io_context is the facilitator to perform I/O operations.
*  To perform those operations, your program will need an I/O obect
*  such as a socket. The io_context is the glue between your I/O object and the
*  operating system I/O services.
*  The future requests (operations) of your I/O object will be forwarded to the
*  io_context.The io_context will call your operating system to perform the
*  wanted operation and will translate the result of the performed operation
*  into an asio error_code, then it will be forwarded back up to your I/O
*  object.
*
*  @link:
*   http://think-async.com/Asio/asio-1.11.0/doc/asio/overview/core/basics.html
*
*  @usefull: Service class is a wrapper for io_context. It is totally based
*  on functionalities provided by Asio. Service is a handler to
*  cover all use cases that a user could encouter performing I/O operations.
*  To ensure thread safety, Service owns a distinct object asio::io_context.
*  Furthermore, to guarantee that the io_context will not exit while operations
*  are running, an asio::io_context::work is implemented as well as a strand
*  object to serialize the execution of handlers and execute them in the
*  according order in wich they have been enqueued.
*
*  @link:
*   http://think-async.com/Asio/asio-1.11.0/doc/asio/reference/io_service__work.html
*
*  @Note: in Asio version 1.11.0, released on Monday 16 February 2015,
*  the io_service object is now named io_context.
*
*/
class Service {
 public:
  // Ctor
  Service()
      : strand_(io_service_),
        stop_(false),
        work_(new asio::io_context::work(io_service_)) {}

  // CopyCtor
  Service(const Service&) = delete;
  // Assignment operator
  Service& operator=(const Service&) = delete;

  // Dtor
  ~Service() {
    io_service_.stop();
    if (thread_.joinable()) thread_.join();
  }

  // runs the service in his dedicated thread.
  void run() {
    if (not stop_)
      if (not thread_.joinable())
        thread_ = std::thread([this]() { io_service_.run(); });
  }

  // asks the I/O service to execute the given handler.
  void post(const std::function<void()>& handler) {
    if (not stop_) io_service_.post(handler);
  }

  // stops the service.
  void stop() {
    if (not stop_) {
      stop_ = true;
      work_.reset();
      if (thread_.joinable())
        thread_.join();
      else {
        io_service_.run();
        io_service_.stop();
      }
    }
  }

  // returns the state of the service.
  bool is_stop() { return stop_; }

  // returns a reference on the I/O service.
  asio::io_context& get() { return io_service_; }

  // returns a reference on the strand object.
  asio::io_context::strand& get_strand() { return strand_; }

  // returns a reference on the smart pointer managing the work obect.
  std::unique_ptr<asio::io_context::work>& get_work() { return work_; }

 private:
  // Dedicated thread to call the run() method.
  std::thread thread_;

  // Indicates if the service is stopped.
  std::atomic<bool> stop_;

  // I/O services.
  asio::io_context io_service_;

  // Asio strand class provides a serialized handler execution, it means that
  // strand class ensures that handlers will be executed according
  // the order wherein they have been enqueued.
  asio::io_context::strand strand_;

  // The work_ variable keeps I/O services alive until there is no unfinished
  // operations remaining. Service owns a smart pointer on the work class to be
  // able to reset it in order to gracefully finish all pending operations.
  std::unique_ptr<asio::io_context::work> work_;
};


/**
*  @brief: Errors handling class
*
*  @description: Error class is a tool to get the code more readable. It owns
*  an asio::error_code variable to deal with error happening with asio objects
*  and gives an access to  many class constructors to handle different
*  kind of error.
*
*  @content:
*     > class User : public std::logic_error
*       handle errors made by Hermes' user
*     > class Connection : public std::runtime_error
*       handle errors relatives to connect operations.
*     > class Write : public ::std::runtime_error
*       handle errors relatives to writting operations.
*     > class Read : public ::std::runtime_error
*       handle errors relatives to reading operations.
*
*/
class Error {
 public:
  Error() {}

  asio::error_code& get() { return error_; }

  bool exist() { return error_ ? true : false; }

  void throw_it() { throw asio::system_error(error_); }

  static void print(const std::string& err) { std::cerr << err << std::endl; }

  // logic errors handler
  class User : public std::logic_error {
   public:
    // ctor as explicit prevents the compiler from using it for implicit
    // conversion
    explicit User(const std::string& error = "error")
        : logic_error("logic error"), message_(error) {}

    virtual ~User() throw() {}

    virtual const char* what() const throw() { return message_.c_str(); }

   private:
    std::string message_;
  };

  // runtime errors handler about connect operations
  class Connection : public std::runtime_error {
   public:
    // ctor as explicit prevents the compiler from using it for implicit
    // conversion
    explicit Connection(const std::string& error = "error")
        : runtime_error("Connect operation"), message_(error) {}

    virtual ~Connection() throw() {}

    virtual const char* what() const throw() { return message_.c_str(); }

   private:
    std::string message_;
  };

  // runtime errors handler about writting operations
  class Write : public std::runtime_error {
   public:
    // ctor as explicit prevents the compiler from using it for implicit
    // conversion
    explicit Write(const std::string& error = "error")
        : runtime_error("Write operation"), message_(error) {}

    virtual ~Write() throw() {}

    virtual const char* what() const throw() { return message_.c_str(); }

   private:
    std::string message_;
  };

  // runtime errors handler about reading operations
  class Read : public std::runtime_error {
   public:
    // ctor as explicit prevents the compiler from using it for implicit
    // conversion
    explicit Read(const std::string& error = "error")
        : runtime_error("Read operation"), message_(error) {}

    virtual ~Read() throw() {}

    virtual const char* what() const throw() { return message_.c_str(); }

   private:
    std::string message_;
  };

 private:
  // Asio error
  asio::error_code error_;
};

}  // namespace core


/**
*  @brief: Hermes network functionalities
*
*  @description: The namespace network contains usefull tools to perform network
*  operations. In order to provide to the user, a library supporting both
*  TCP/UDP protocol, a tcp and udp sockets have been wrapped respectively in
*  a Stream and a Datagram class.
*  Stream and Datagram have been designed to serialize operations on I/O object
*  according their protocol.
*  Both of those wrappers need a Service object to be able to to call the I/O
*  services of your operating system.
*
*  @require: Hermes::core
*
*/
namespace network {

/**
*   @brief: an asio::tcp::ip::socket wrapper to manage and serialize operations
*   on the socket.
*
*   @description: Stream provides a TCP "session", by this i mean, it offers a
*   dedicated object wich owns a tcp socket and represent a new tcp connection.
*   Stream is a serialized way to handle TCP operations on a I/O object.
*   Thinking asynchronously, Stream needs a reference on a Service to be
*   constructed. Stream will rest on the given service to perform the future
*   asynchronous operations made by the user.
*   shared_ptr and enable_shared_from_this are used to keep the Stream object
*   alive as long as there is an operation that refers to it.
*
*/
class Stream : public std::enable_shared_from_this<Stream> {
 public:
  typedef std::shared_ptr<Stream> session;

  // Creates a new TCP session.
  static session new_session(core::Service& service) {
    return session(new Stream(service));
  }

  // synchronous connection to the given endpoint.
  // @param: a valid endpoint.
  void connect(const asio::ip::tcp::endpoint& endpoint) {
    core::Error error;

    socket_.connect(endpoint, error.get());
    if (error.exist()) error.throw_it();
    connected_ = true;
  }

  // asynchronous connection to the given endpoint
  // a callback can be provided, it will be executed once the connection
  // established.
  //
  // @param:
  //    - endpoint
  //    - a reference on a std::function returning void and taking a reference
  //      on the stream as parameter
  //
  // @code: c++
  //  Service service;
  //  Stream::session = Stream::new_session(service);
  //  asio::ip::tcp::endpoint; // get an endpoint by resolving host:port
  //                           // cf: asio documentation.
  //
  //  session->async_connect(endpoint, [](Stream& stream){
  //     // Do some stuff
  //  });
  //
  // @endcode
  void async_connect(const asio::ip::tcp::endpoint& endpoint,
                     const std::function<void(Stream&)>& callback = nullptr) {
    if (connected_) return;

    std::condition_variable condvar;
    std::atomic_bool notified(false);

    socket_.async_connect(endpoint, [&](const asio::error_code& error) {

      if (error) throw asio::system_error(error);
      connected_ = true;
      if (callback) callback(*this);
      notified = true;
      // notify that the connection has succeeded
      condvar.notify_one();
    });

    std::mutex mutex;
    std::unique_lock<std::mutex> lock(mutex);
    service_.run();
    if (not notified) condvar.wait(lock);
  }

  // Stops the stream.
  // @Note: does not stop the service.
  void disconnect() {
    if (not connected_) return;

    connected_ = false;

    // To prevent some concurrency issues, the shutdown and close calls
    // are locked, waiting to be notified that one thread has completed the
    // tasks.
    // First thread to come up here, changes the boolean to false and runs the
    // function object to close/shutdown the socket and he notifies that the job
    // is completed. Once that is done, next thread to come up here will have
    // the boolean connected_ set with false as value and the function will
    // return immediately.
    std::mutex mutex;
    std::condition_variable condvar;
    std::unique_lock<std::mutex> lock(mutex);

    std::atomic_bool notified(false);
    service_.get_strand().post([this, &condvar, &notified]() {
      core::Error error;
      // asio error_code provided to ensure that no exception will be thrown.
      socket_.shutdown(asio::ip::tcp::socket::shutdown_both);
      socket_.close(error.get());
      notified = true;
      condvar.notify_one();
    });
    if (not notified) condvar.wait(lock);
  }

  // Synchronous send of amount of data.
  // @param: the message to send.
  std::size_t send(const std::string& message) {
    core::Error error;
    std::mutex mutex;

    std::lock_guard<std::mutex> lock(mutex);
    auto bytes = asio::write(
        socket_, asio::buffer(message.data(), message.size()), error.get());

    if (error.exist()) error.throw_it();

    if (bytes != message.size())
      throw core::Error::Write(
          "Unexpected error occurred: asio::write failed. All data have not "
          "been sent.");
    return bytes;
  }

  // asynchronous send of amount of data
  // Asks to strand to execute an asynchronous write on the socket.
  void async_send(const std::string& message) {
    // strand serializes the given handler
    service_.get_strand().post(
        std::bind(&Stream::async_send_handler, shared_from_this(), message));
  }

  // Synchronous receive.
  // The size of the data is defined by core::BUFFER_SIZE.
  // Each message including his size > core::BUFFER_SIZE will be imcompleted.
  std::string receive() {
    core::Error error;
    std::mutex mutex;
    char buffer[core::BUFFER_SIZE] = {0};

    std::lock_guard<std::mutex> lock(mutex);
    socket_.read_some(asio::buffer(buffer, core::BUFFER_SIZE), error.get());

    if (error.exist()) error.throw_it();

    if (not std::string(buffer).size())
      throw core::Error::Read(
          "Unexpected error occurred. asio::ip::tpc::socket::read_some failed. "
          "0 bytes received.");
    return std::string(buffer);
  }

  // asynchronous receive of data
  // Asks to strand to execute an asynchronous read on the socket.
  void async_receive() {
    // strand serializes the given handler
    service_.get_strand().post(
        std::bind(&Stream::async_receive_handler, shared_from_this()));
  }

  // sets the callback wich will be invoked by the asynchronous send.
  void set_write_handler(
      const std::function<void(std::size_t, Stream&)>& callback) {
    write_handler_ = callback;
  }

  // sets the callback wich will be invoked by the asynchronous receive.
  void set_read_handler(
      const std::function<void(std::string, Stream&)>& callback) {
    read_handler_ = callback;
  }

  // returns if the stream is connected.
  bool is_connected() { return connected_; }

  // returns the reference on the service used by the session.
  core::Service& service() { return service_; }

  // returns a reference on the socket used by the session.
  asio::ip::tcp::socket& socket() { return socket_; }

 private:
  // ctor
  Stream(core::Service& service)
      : service_(service),
        connected_(false),
        socket_(service.get()),
        read_handler_(nullptr),
        write_handler_(nullptr) {
    std::memset(buffer_, 0, core::BUFFER_SIZE);
  }

  // Performs the asio::async_write operation.
  // this function is post through the strand object to ensure that it
  // will be invoked once and at the good moment.
  void async_send_handler(const std::string& message) {
    auto roxanne(shared_from_this());
    asio::async_write(
        socket_, asio::buffer(message),
        service_.get_strand().wrap(
            [this, roxanne](const asio::error_code& error, std::size_t bytes) {

              if (error) throw asio::system_error(error);

              if (not bytes)
                throw core::Error::Write(
                    "Unexpected error occurred. asio::async_write failed;");

              std::mutex mutex;
              // lock the execution of the handler to guarantee the thread
              // safety.
              std::lock_guard<std::mutex> lock(mutex);
              if (write_handler_) write_handler_(bytes, *shared_from_this());
            }));
  }

  // Performs an asynchronous read on the socket.
  // this function is post through the strand object to ensure that it
  // will be invoked once and at the good moment.
  void async_receive_handler() {
    auto roxanne(shared_from_this());
    socket_.async_read_some(
        asio::buffer(buffer_, core::BUFFER_SIZE),
        service_.get_strand().wrap(
            [this, roxanne](const asio::error_code& error, std::size_t bytes) {

              if (error) throw asio::system_error(error);

              if (not bytes)
                throw core::Error::Read(
                    "Unexpected error occurred. asio::async_read failed.");

              std::mutex mutex;
              std::lock_guard<std::mutex> lock(mutex);
              // lock the execution of the handler to guarantee the thread
              // safety.
              if (read_handler_)
                read_handler_(std::string(buffer_), *shared_from_this());
              std::memset(buffer_, 0, core::BUFFER_SIZE);
            }));
  }

  // A reference on the service to perform I/O operations.
  core::Service& service_;

  // Thread safe boolean to know if the stream is connected.
  std::atomic<bool> connected_;

  // TCP socket.
  asio::ip::tcp::socket socket_;

  // Buffer.
  char buffer_[core::BUFFER_SIZE];

  // Asynchronous receive handler.
  std::function<void(std::string, Stream&)> read_handler_;

  // Asynchronous send handler.
  std::function<void(std::size_t, Stream&)> write_handler_;
};

}  // namespace network


/**
*   @brief: Hermes protobuf operations.
*
*
*   @description: The protobuf namespace contains the Hermes operations about
*   sending/receiving serialized data through socket using the Google protocols
*   Buffers serialization protocol. Those operations use TCP protocol.
*
*   @require: Hermes::core
*             Hermes::network::Stream
*
*/
namespace protobuf {

// synchronous send of a serialized protobuf message
template <typename T>
std::size_t send(const std::string& host, const std::string& port,
                 const T& message) {
  core::Service service;
  std::size_t bytes = 0;
  std::string protobuf("");
  auto session = network::Stream::new_session(service);

  try {
    session->service().run();
    message.SerializeToString(&protobuf);
    asio::ip::tcp::resolver resolver(service.get());
    session->connect(
        *resolver.resolve(asio::ip::tcp::resolver::query(host, port)));
    bytes = session->send(protobuf);
  } catch (std::exception& e) {
    core::Error::print(e.what());
  }
  return bytes;
}

// synchronous receive of a protobuf message
// message is constructed from a serialized string.
template <typename T>
T receive(const std::string& port) {
  core::Service service;
  std::string received("");
  auto session = network::Stream::new_session(service);

  try {
    asio::ip::tcp::acceptor acceptor(
        service.get(),
        asio::ip::tcp::endpoint(asio::ip::tcp::v4(), std::stoi(port)));
    acceptor.set_option(asio::ip::tcp::acceptor::reuse_address(true));
    acceptor.accept(session->socket());
    received = session->receive();
  } catch (std::exception& e) {
    core::Error::print(e.what());
  }
  T result;
  result.ParseFromString(received);
  return result;
}

// asynchronous send of a serialized protobuf message
// a callback could be provided like a std::function or a lambda, as parameter.
// the callback will be invoked when the asynchronous send will be performed.
template <typename T>
void async_send(const std::string& host, const std::string& port,
                const T& message,
                const std::function<void(std::size_t)>& callback = nullptr) {
  core::Service service;
  std::string protobuf("");
  auto session = network::Stream::new_session(service);
  auto handler = [callback](std::size_t bytes, network::Stream& session) {
    if (callback) callback(bytes);
  };

  try {
    message.SerializeToString(&protobuf);
    asio::ip::tcp::resolver resolver(service.get());
    session->async_connect(
        *resolver.resolve(asio::ip::tcp::resolver::query(host, port)));
    session->set_write_handler(handler);
    session->async_send(protobuf);
    session->disconnect();
    session->service().stop();
  } catch (std::exception& e) {
    core::Error::print(e.what());
  }
}

// asynchronous receive of a serialized protobuf message
// a callback could be provided like a std::function or a lambda, as parameter.
// the callback will be invoked when the asynchronous receive will be performed.
template <typename T>
void async_receive(const std::string& port,
                   const std::function<void(T)>& callback = nullptr) {
  core::Service service;
  std::string received("");
  auto session = network::Stream::new_session(service);
  auto handler = [callback](std::string received, hermes::network::Stream& s) {
    T result;
    result.ParseFromString(received);
    if (callback) callback(result);
  };

  try {
    asio::ip::tcp::acceptor acceptor(service.get());
    asio::ip::tcp::endpoint endpoint(asio::ip::tcp::v4(), std::stoi(port));
    acceptor.open(endpoint.protocol());
    acceptor.set_option(asio::ip::tcp::acceptor::reuse_address(true));
    acceptor.bind(endpoint);
    acceptor.listen();
    acceptor.async_accept(session->socket(),
                          [&](const asio::error_code& error) {
                            if (error) throw asio::system_error(error);
                            session->set_read_handler(handler);
                            session->async_receive();
                          });
    session->disconnect();
    session->service().stop();
  } catch (std::exception& e) {
    core::Error::print(e.what());
  }
}

}  // namespace protobuf

}  // namespace hermes
