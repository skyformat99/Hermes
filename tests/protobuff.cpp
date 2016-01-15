#include "Hermes.hpp"
#include "Communication.pb.h"

#include <thread>
#include <iostream>
#include <unistd.h>
#include <assert.h>

using namespace Hermes;

void send_routine(const com::Message& message) {
  std::string serialized;

  std::this_thread::sleep_for(std::chrono::milliseconds(300));
  message.SerializeToString(&serialized);
  auto size = protobuf::send<com::Message>("127.0.0.1", "8246", message);
  assert(size == serialized.size());
}

void receive_routine() {
  auto test = protobuf::receive<com::Message>("8246");
  assert(test.name() == "name");
  assert(test.object() == "object");
  assert(test.from() == "from");
  assert(test.to() == "to");
  assert(test.msg() == "msg");
}

void test_protobuf_synchronous_operations() {
  std::cout << "testing synchronous operations [protobuf]" << std::endl;
  com::Message message;

  message.set_name("name");
  message.set_object("object");
  message.set_from("from");
  message.set_to("to");
  message.set_msg("msg");

  std::thread thread_receive(receive_routine);
  std::thread thread_send(send_routine, message);
  thread_send.join();
  thread_receive.join();
  std::cout << "-> test synchronous operations [ok]." << std::endl;
}

void test_protobuf_asynchronous_operations() {
  std::cout << "testing asynchronous operations [protobuf]" << std::endl;
  com::Message message;
  com::Message response;

  message.set_name("name: ok");
  message.set_object("object: ok");
  message.set_from("from: ok");
  message.set_to("to: ok");
  message.set_msg("msg: ok");

  std::thread thread_receive([&]() {
    protobuf::async_receive<com::Message>("25555", [&](com::Message response) {
      assert(response.name() == "name: ok");
      assert(response.object() == "object: ok");
      assert(response.from() == "from: ok");
      assert(response.to() == "to: ok");
      assert(response.msg() == "msg: ok");
    });
  });

  std::thread thread_send([&]() {
    std::this_thread::sleep_for(std::chrono::milliseconds(300));
    protobuf::async_send<com::Message>("127.0.0.1", "25555", message,
                                       [&](std::size_t bytes) {
       assert(bytes == 49);
      });
  });

  thread_send.join();
  thread_receive.join();
  std::cout << "-> test asynchronous operations [ok]." << std::endl;
}
