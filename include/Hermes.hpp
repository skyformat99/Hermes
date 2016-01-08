#pragma once

#include <ctime>
#include <chrono>
#include <string>
#include <memory>
#include <iostream>
#include <stdexcept>
#include <algorithm>
#include <functional>

#include "google/protobuf/message.h"
#include "asio.hpp"

namespace Hermes {

using namespace asio::ip;

namespace protobuf {

using namespace google::protobuf;

template <typename T>
std::size_t send(const std::string& host, const std::string& port,
                 std::shared_ptr<T> message) {
  std::string serialized;
  char buffer[2048] = {0};
  asio::io_context io_service;
  asio::error_code error = asio::error::host_not_found;

  if (not message)
    throw std::logic_error(
        "[protobuff]: given nullptr as parameter (message).");

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
        "[protobuf] Unexpected error occurred: accept failed to accept "
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

  auto response = buffer;
  auto result = std::make_shared<T>();
  result->ParseFromString(response);
  return result;
}

typedef std::function<void()> callback;
template <typename T>
void async_send(const std::string& host, const std::string& port,
                std::shared_ptr<T> message, const callback& handler = nullptr) {
  char buffer[2048] = {0};
  std::string serialized;
  asio::io_context io_service;

  if (not message)
    throw std::logic_error(
        "[protobuff]: given nullptr as parameter (message).");

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
          "[protobuf] Unexpected error occurred: 0 bytes sent"
        );

      if (handler) handler();

    });

  });
  io_service.run();
}

typedef std::function<void(const std::string&)> rcallback;
template <typename T>
void async_receive(const std::string& port, std::shared_ptr<T> message,
                   const rcallback& handler = nullptr) {
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

      if (error && error != asio::error::eof) {
        acceptor.close();
        socket.close();
        throw asio::system_error(error);
      }

      if (not bytes)
        throw std::runtime_error(
          "[protobuf] Unexpected error occurred: 0 bytes received"
        );

      if (handler) handler(std::string(buffer));

      else {
        std::string result(buffer);
        message->ParseFromString(result);
      }

    });

  });
  io_service.run();
}

}  // namespace protobuf
}  // namespace Hermes
