#pragma once

#include <string>
#include <iostream>

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
  Service() : work_(io_service_) {}

  ~Service() {}

 private:
  // I/O services
  asio::io_context io_service_;
  // keep I/O services alive until there is no unfinished operations remaining
  asio::io_context::work work_;
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
class Error {
 public:
  Error() {}

  asio::error_code& get() { return error_; }

  // asio's error class
  asio::error_code error_;
};

}  // namespace core

}  // namespace hermes
