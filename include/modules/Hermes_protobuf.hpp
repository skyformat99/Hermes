#pragma once

#include <string>
#include <iostream>
#include <stdexcept>
#include <functional>

#include <assert.h>

#include "google/protobuf/message.h"
#include "asio.hpp"

namespace Hermes {

using namespace asio::ip;

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
