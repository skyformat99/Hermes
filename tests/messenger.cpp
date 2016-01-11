#include <thread>
#include <iostream>
#include <unistd.h>
#include <assert.h>

#include "Hermes.hpp"

using namespace Hermes;
using namespace asio::ip;

void test_session() {
  asio::io_context io_service;
  asio::ip::tcp::socket socket(io_service);
  Session<asio::ip::tcp::socket> session(socket);

  assert(session.is_socket_unused());
  assert(not session.is_ready_for_reading());
  assert(not session.is_ready_for_writting());

  session.set_state_to_socket(Hermes::READING);
  assert(not session.is_socket_unused());
  assert(session.is_ready_for_reading());
  assert(not session.is_ready_for_writting());

  session.set_state_to_socket(Hermes::WRITTING);
  assert(not session.is_socket_unused());
  assert(not session.is_ready_for_reading());
  assert(session.is_ready_for_writting());

  session.activate_option("deadline");
  assert(session.is_option_activated("deadline"));
  assert(not session.is_option_activated("state"));
  assert(not session.is_option_activated("heartbeat"));
}

void test_stream() {
  asio::io_context io_service;

  auto stream = Stream<tcp::socket>::create(io_service);
  assert(stream->session().is_socket_unused());
}

void test_messenger_constructor()
{
  assert(std::make_shared<Hermes::Messenger>("client", "tcp", true, "8080"));
  assert(new Hermes::Messenger("client", "udp", true, "4444", "192.168.1.78"));

  auto toto = std::make_shared<Messenger>("1", "2", true, "3")->get_messenger();
}
