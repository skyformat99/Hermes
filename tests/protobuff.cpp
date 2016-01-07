#include "Hermes.hpp"
#include "Communication.pb.h"

#include <thread>
#include <iostream>
#include <unistd.h>
#include <assert.h>

using namespace Hermes;

void send_routine(std::shared_ptr<com::Message> message) {
  auto size = protobuf::send<com::Message>("127.0.0.1", "1234", message);
  assert(size != 0);
}

void receive_routine() {
  auto test = protobuf::receive<com::Message>("1234");
  assert(test->name() == "Tommy");
  assert(test->object() == "Test");
  assert(test->from() == "Hermes");
  assert(test->to() == "The World");
  assert(test->msg() == "Hope it works!");
}

void test_protobuf_synchronous_operations()
{
  std::string serialized;
  auto protobuf_to_send = std::make_shared<com::Message>();

  protobuf_to_send->set_name("Tommy");
  protobuf_to_send->set_object("Test");
  protobuf_to_send->set_from("Hermes");
  protobuf_to_send->set_to("The World");
  protobuf_to_send->set_msg("Hope it works!");

  std::thread thread_receive(receive_routine);
  sleep(1);
  std::thread thread_send(send_routine, protobuf_to_send);
  thread_send.join();
  thread_receive.join();
}

void test_protobuf_asynchronous_operations() {
  
}
