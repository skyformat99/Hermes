#include "Hermes.hpp"
#include "Communication.pb.h"

#include <thread>
#include <iostream>
#include <unistd.h>
#include <assert.h>

using namespace Hermes;

void send_routine(std::shared_ptr<com::Message> message) {
  auto size = protobuf::send<com::Message>("127.0.0.1", "8246", message);
  assert(size != 0);
  std::cout << "protobuf sent :)" << "\n";
}

void receive_routine() {
  auto test = protobuf::receive<com::Message>("8246");
  assert(test->name() == "Tommy");
  assert(test->object() == "Test");
  assert(test->from() == "Hermes");
  assert(test->to() == "The World");
  assert(test->msg() == "Hope it works!");
  std::cout << "It worked :)" << "\n";
}

void test_protobuf_synchronous_operations() {
  auto protobuf_to_send = std::make_shared<com::Message>();

  protobuf_to_send->set_name("Tommy");
  protobuf_to_send->set_object("Test");
  protobuf_to_send->set_from("Hermes");
  protobuf_to_send->set_to("The World");
  protobuf_to_send->set_msg("Hope it works!");

  std::thread thread_receive(receive_routine);
  std::thread thread_send(send_routine, protobuf_to_send);
  thread_send.join();
  thread_receive.join();
}

void test_protobuf_asynchronous_operations() {
  auto protobuf_to_send = std::make_shared<com::Message>();
  auto protobuf_to_receive = std::make_shared<com::Message>();

  protobuf_to_send->set_name("1");
  protobuf_to_send->set_object("2");
  protobuf_to_send->set_from("3");
  protobuf_to_send->set_to("4");
  protobuf_to_send->set_msg("5");

  std::thread thread_receive([&]() {
    protobuf::async_receive<com::Message>("25555", protobuf_to_receive,
                                          [&](const std::string& res) {
      protobuf_to_receive->ParseFromString(res);
      assert(protobuf_to_receive->name() == "1");
      assert(protobuf_to_receive->object() == "2");
      assert(protobuf_to_receive->from() == "3");
      assert(protobuf_to_receive->to() == "4");
      assert(protobuf_to_receive->msg() == "5");
      std::cout << "protobuf received :)" << "\n";
    });
  });

  std::thread thread_send([&]() {
    protobuf::async_send<com::Message>("127.0.0.1", "25555", protobuf_to_send,
                                       [&]() {
      std::cout << "do some stuff :)" << std::endl;
    });
  });

  thread_send.join();
  thread_receive.join();
}
