#include <thread>
#include <iostream>
#include <unistd.h>
#include <assert.h>

#include "Hermes.hpp"
#include "Communication.pb.h"

using namespace Hermes;

void test_session() {
  std::cout << "testing session [Messenger]" << std::endl;
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

  session.activate_option("state");
  assert(session.is_option_activated("deadline"));
  assert(session.is_option_activated("state"));
  assert(not session.is_option_activated("heartbeat"));

  session.activate_option("heartbeat");
  assert(session.is_option_activated("deadline"));
  assert(session.is_option_activated("state"));
  assert(session.is_option_activated("heartbeat"));

  assert(session.get_heartbeat_message() == "<3");
  session.set_heartbeat_message("test");
  assert(session.get_heartbeat_message() == "test");

  session.stop();
  assert(not socket.is_open());
  std::cout << "-> test session: [ok]." << std::endl;
}

void test_stream() {
  std::cout << "testing stream [Messenger]" << std::endl;
  asio::io_context io_service;

  auto stream = Stream<tcp::socket>::create(io_service);
  assert(stream->session().is_socket_unused());
  assert(not stream->session().is_ready_for_reading());
  assert(not stream->session().is_ready_for_writting());

  stream->session().set_state_to_socket(Hermes::READING);
  assert(not stream->session().is_socket_unused());
  assert(stream->session().is_ready_for_reading());
  assert(stream->session().is_ready_for_writting());

  stream->session().set_state_to_socket(Hermes::WRITTING);
  assert(not stream->session().is_socket_unused());
  assert(not stream->session().is_ready_for_reading());
  assert(stream->session().is_ready_for_writting());

  stream->session().activate_option("deadline");
  assert(stream->session().is_option_activated("deadline"));
  assert(not stream->session().is_option_activated("state"));
  assert(not stream->session().is_option_activated("heartbeat"));

  stream->session().activate_option("state");
  assert(stream->session().is_option_activated("deadline"));
  assert(stream->session().is_option_activated("state"));
  assert(not stream->session().is_option_activated("heartbeat"));

  stream->session().activate_option("heartbeat");
  assert(stream->session().is_option_activated("deadline"));
  assert(stream->session().is_option_activated("state"));
  assert(stream->session().is_option_activated("heartbeat"));

  assert(stream->session().get_heartbeat_message() == "<3");
  stream->session().set_heartbeat_message("test");
  assert(stream->session().get_heartbeat_message() == "test");

  stream->session().stop();
  assert(not stream->socket().is_open());
  std::cout << "-> test stream [ok]." << std::endl;
}

void test_messenger_tcp_client()
{
  std::cout << "testing tcp client [Messenger]" << std::endl;
  auto client = new Messenger("client", "tcp", true, "6666", "127.0.0.1");
  assert(client);
  client->run();
  client->send("it works :)");
  auto response = client->receive() ;
  std::cout << response << "\n";

  std::thread uno([&](){
      client->async_receive([](const std::string& response){
        std::cout << "DEBUG: " << response << "\n";
      });
  });

  std::thread dos([&](){
    std::this_thread::sleep_for(std::chrono::seconds(1));
    client->async_send(" StarK! ", [](std::size_t bytes){
      std::cout << "message sent. number of bytes: " + bytes;
    });
  });

  std::this_thread::sleep_for(std::chrono::seconds(2));
  client->disconnect();
  uno.join();
  dos.join();
  std::cout << "-> test tcp client [ok]." << std::endl;
}
