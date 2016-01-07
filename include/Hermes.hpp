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

  tcp::resolver resolver(io_service);
  tcp::socket socket(io_service);
  auto endpoint = resolver.resolve(tcp::resolver::query(host, port));

  message->SerializeToString(&serialized);
  std::strcpy(buffer, serialized.c_str());
  asio::connect(socket, endpoint);
  asio::write(socket, asio::buffer(buffer, serialized.size()));
  socket.close();
  io_service.run();
  return serialized.size();
}

template <typename T>
std::shared_ptr<T> receive(const std::string& port) {
  asio::error_code error;
  char buffer[2048] = {0};
  asio::io_context io_service;

  tcp::socket socket(io_service);
  tcp::acceptor acceptor(io_service, tcp::endpoint(tcp::v4(), std::stoi(port)));
  acceptor.accept(socket);
  socket.read_some(asio::buffer(buffer), error);
  socket.close();

  if (error == asio::error::eof)
    return nullptr;
  else if (error)
    throw asio::system_error(error);

  auto response = buffer;
  auto result = std::make_shared<T>();
  result->ParseFromString(response);
  io_service.run();
  return result;
}

typedef std::function<void()> callback;
template <typename T>
void async_send(const std::string& host, const std::string& port,
                std::shared_ptr<T> message, const callback& handler =
                nullptr) {
  std::string serialized;
  asio::io_context io_service;

  tcp::socket socket(io_service);
  tcp::resolver resolver(io_service);
  message->SerializeToString(&serialized);
  auto endpoint = resolver.resolve(tcp::resolver::query(host, port));
  socket.async_connect(endpoint->endpoint(),
                       [&](const asio::error_code& error) {

    char buffer[2048] = {0};

    if (error) {
      socket.close();
      return;
    }

    std::strcpy(buffer, serialized.c_str());
    asio::async_write(socket, asio::buffer(buffer, serialized.size()),
                      [&](const asio::error_code& error, std::size_t bytes) {

      if (not bytes || error) {
        socket.close();
        return;
      }

      if (handler) handler();

    });

  });
  io_service.run();
}

}  // namespace protobuf
}  // namespace Hermes
