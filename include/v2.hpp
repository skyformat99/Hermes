#pragma once

#include <mutex>
#include <chrono>
#include <memory>
#include <string>
#include <thread>
#include <vector>
#include <iostream>
#include <stdexcept>
#include <functional>

#include "asio.hpp"
#include "google/protobuf/message.h"

namespace hermes {

/**
*   @brief: Hermes core functionalities
*
*   @description: core namespace contains various classes to provide basic
*   features such as I/O services or errors handling.
*
*
*
*/
namespace core {

/**
*  @brief: Your program's link to your operating system I/O services.
*
*  @description: Asio's io_context is the facilitator to perform I/O operations.
*  To perform those operations, your program will need an I/O obect
*  such as a socket. The io_context is the glue between your I/O object and the
*  operating system I/O services.
*  The future requests (operations) of your I/O object will be forwarded to the
*  io_context. io_context will call your operating system to perform the wanted
*  operation and will translate the result of the performed operation into an
*  asio error_code, then it will be forwarded back up to your I/O object.
*
*  @link:
*   http://think-async.com/Asio/asio-1.11.0/doc/asio/overview/core/basics.html
*
*  @usefull: Service class is a wrapper for io_context. It is totally based
*  on functionalities provided by Asio. Service is a handler to
*  cover all use cases that a user could encouter performing I/O operations.
*  To ensure thread safety, Service owns a distinct object asio::io_context.
*  Furthermore, to guarantee that the io_context will not exit while operations
*  are running, an asio::io_context::work is implemented.
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
      : strand_(io_service_), work_(new asio::io_context::work(io_service_)) {}

  // CopyCtor
  Service(const Service&) = delete;
  // Move assignment operator
  Service& operator=(const Service&) = delete;

  // Dtor
  ~Service() {
    if (thread_.joinable()) {
      io_service_.stop();
      thread_.join();
    }
  }

  void run() {
    if (not thread_.joinable())
      thread_ = std::thread([this]() { io_service_.run(); });
  }

  void post(const std::function<void()>& handler) { io_service_.post(handler); }

  asio::io_context& get() { return io_service_; }

  asio::io_context::strand& get_strand() { return strand_; }

  std::unique_ptr<asio::io_context::work>& get_work() { return work_; }

 private:
  // Dedicated thread to call the run() method.
  std::thread thread_;

  // I/O services.
  asio::io_context io_service_;

  // Asio strand class provides a serialized handler execution, it means that
  // strand class ensures that handlers will be executed according
  // the order wherein they have been queued.
  asio::io_context::strand strand_;

  // The work_ variable keeps I/O services alive until there is no unfinished
  // operations remaining. Service owns a smart pointer on the work class to be
  // able to reset it in order to gracefully finish all pending operations.
  std::unique_ptr<asio::io_context::work> work_;
};

/**
*  @brief: Errors handling class
*
*  @description: Error class is a tool to get the code more readable. It owns an
*  asio::error_code variable to deal with error happening with asio object and
*gives
*  an access to  many class constructors to handle different kind of error.
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

  static void throw_asio_error(asio::error_code& error) {
    throw asio::system_error(error);
  }

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

}  // namespace hermes
