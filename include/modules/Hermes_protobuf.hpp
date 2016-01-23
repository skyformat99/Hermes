#pragma once

#include <string>
#include <iostream>
#include <stdexcept>
#include <functional>

#include <assert.h>
#undef NDEBUG

#include "google/protobuf/message.h"
#include "asio.hpp"

#define BUFFER_SIZE 2048

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
*  unknown behavior will happen.
*
* @protocol:
*   TCP
*
* @see:
*   design:
        https://github.com/TommyStarK/Hermes/blob/master/DESIGN.md
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
    socket.shutdown(asio::ip::tcp::socket::shutdown_both);
    socket.close();
    throw std::runtime_error("[protobuf] Connection to host: " + host +
                             " port: " + port + " failed.");
  }

  asio::write(socket, asio::buffer(buffer, serialized.size()), error);

  if (error) {
    socket.shutdown(asio::ip::tcp::socket::shutdown_both);
    socket.close();
    throw asio::system_error(error);
  }

  socket.shutdown(asio::ip::tcp::socket::shutdown_both);
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
  acceptor.set_option(asio::ip::tcp::acceptor::reuse_address(true));
  acceptor.accept(socket, error);

  if (error) {
    socket.shutdown(asio::ip::tcp::socket::shutdown_both);
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
    socket.shutdown(asio::ip::tcp::socket::shutdown_both);
    socket.close();
    acceptor.close();
    throw asio::system_error(error);
  }

  T result;
  result.ParseFromString(std::string(buffer));
  socket.shutdown(asio::ip::tcp::socket::shutdown_both);
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
      socket.shutdown(asio::ip::tcp::socket::shutdown_both);
      socket.close();
      throw std::runtime_error("[protobuf] Connection to host: " + host +
                               " port: " + port + " failed.");
    }

    std::strcpy(buffer, serialized.c_str());
    asio::async_write(socket, asio::buffer(buffer, serialized.size()),
                      [&](const asio::error_code& error, std::size_t bytes) {

      if (error) {
        socket.shutdown(asio::ip::tcp::socket::shutdown_both);
        socket.close();
        throw std::system_error(error);
      }

      if (not bytes) {
        socket.shutdown(asio::ip::tcp::socket::shutdown_both);
        socket.close();
        throw std::runtime_error(
            "[protobuf] Unexpected error occurred: 0 bytes sent");
      }

      if (callback) callback(bytes);
      socket.shutdown(asio::ip::tcp::socket::shutdown_both);
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
      socket.shutdown(asio::ip::tcp::socket::shutdown_both);
      socket.close();
    });
  });
  std::thread([&]() { io_service.run(); }).join();
}

}  // namespace protobuf
}  // namespace Hermes
