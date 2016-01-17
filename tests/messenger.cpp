#include <thread>
#include <iostream>
#include <unistd.h>

#include "Communication.pb.h"
#include "Hermes.hpp"

using namespace Hermes;

void test_session() {
  std::cout << "[Messenger] testing session." << std::endl;
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
  std::cout << "-> test session: [ok]." << std::endl;
}

void test_stream() {
  std::cout << "[Messenger] testing stream." << std::endl;
  asio::io_context io_service;

  auto stream = Stream<tcp::socket>::create(io_service);

  assert(stream);
  assert(not stream->socket().is_open());

  stream->socket().open(asio::ip::tcp::v4());
  assert(stream->socket().is_open());

  assert(stream->session().is_socket_unused());
  assert(not stream->session().is_ready_for_reading());
  assert(not stream->session().is_ready_for_writting());

  stream->session().set_state_to_socket(Hermes::READING);
  assert(not stream->session().is_socket_unused());
  assert(stream->session().is_ready_for_reading());
  assert(not stream->session().is_ready_for_writting());

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

  stream->stop();
  assert(not stream->socket().is_open());
  std::cout << "-> test stream [ok]." << std::endl;
}

void test_tcp_protocol() {

  std::thread server([](){
    std::cout << "[Messenger] testing tcp server." << std::endl;
    auto server = new Messenger("server", "tcp", true, "7777");
    assert(server);
    std::cout << "-> test tcp server [ok]." << std::endl;
  });

  std::thread client([](){

    std::cout << "[Messenger] testing tcp client." << std::endl;
    auto client = new Messenger("client", "tcp", true, "6666", "127.0.0.1");
    assert(client);

    // client->run([](){
    //   std::cout << "Connected :) " << std::endl;
    // });
    //
    // client->async_send("123456789\n", [](std::size_t bytes) {
    //   std::cout << "sent: " << bytes << std::endl;
    // });
    //
    // client->async_receive([](std::string response){
    //   std::cout << "response: " << response << std::endl;
    // });
    //
    // client->disconnect([](){
    //   std::cout << "Deconnected :)" << std::endl;
    // });
    std::cout << "-> test tcp client [ok]." << std::endl;

  });


  client.join();
  server.join();
}

void test_udp_protocol() {

  std::thread server([](){
    std::cout << "[Messenger] testing udp server." << std::endl;
    auto server = new Messenger("server", "udp", true, "4444");
    assert(server);
    std::cout << "-> test udp server [ok]." << std::endl;
  });

  std::thread client([](){
    std::cout << "[Messenger] testing udp client." << std::endl;
    auto client = new Messenger("client", "udp", true, "4444", "127.0.0.1");
    assert(client);
    std::cout << "-> test udp client [ok]." << std::endl;
  });

  server.join();
  client.join();
}
