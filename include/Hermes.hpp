#pragma once

#include <ctime>
#include <chrono>
#include <string>
#include <memory>
#include <iostream>
#include <stdexcept>
#include <algorithm>

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
  tcp::resolver::query query(host, port);
  tcp::socket socket(io_service);
  auto endpoint = resolver.resolve(query);

  message->SerializeToString(&serialized);
  std::strcpy(buffer, serialized.c_str());
  asio::connect(socket, endpoint);
  asio::write(socket, asio::buffer(buffer, serialized.size()));
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

  if (error == asio::error::eof)
    return nullptr;
  else if (error)
    throw asio::system_error(error);

  auto response = buffer;
  auto result = std::make_shared<T>();
  result->ParseFromString(response);
  return result;
}

}
}
