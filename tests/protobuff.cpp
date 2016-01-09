#include "Hermes.hpp"
#include "Communication.pb.h"

#include <thread>
#include <iostream>
#include <unistd.h>
#include <assert.h>

using namespace Hermes;

void send_routine(std::shared_ptr<com::Message> message) {
  std::string serialized;

  message->SerializeToString(&serialized);
  auto size = protobuf::send<com::Message>("127.0.0.1", "8246", message);
  assert(size == serialized.size());
  std::cout << "message sent :)" << "\n";
}

void receive_routine() {
  auto test = protobuf::receive<com::Message>("8246");
  assert(test->name() == "name");
  assert(test->object() == "object");
  assert(test->from() == "from");
  assert(test->to() == "to");
  assert(test->msg() == "msg");
  std::cout << "response received :)" << "\n";
}

void test_protobuf_synchronous_operations() {
  auto message = std::make_shared<com::Message>();

  message->set_name("name");
  message->set_object("object");
  message->set_from("from");
  message->set_to("to");
  message->set_msg("msg");

  std::thread thread_receive(receive_routine);
  std::thread thread_send(send_routine, message);
  thread_send.join();
  thread_receive.join();
}

void test_protobuf_asynchronous_operations() {
  auto message = std::make_shared<com::Message>();
  auto response = std::make_shared<com::Message>();

  message->set_name("name: ok");
  message->set_object("object: ok");
  message->set_from("from: ok");
  message->set_to("to: ok");
  message->set_msg("msg: ok");

  std::thread thread_receive([&]() {
    protobuf::async_receive<com::Message>("25555", response.get(),
                                          [&](const std::string& res) {
      response->ParseFromString(res);
      assert(response->name() == "name: ok");
      assert(response->object() == "object: ok");
      assert(response->from() == "from: ok");
      assert(response->to() == "to: ok");
      assert(response->msg() == "msg: ok");
      std::cout << "string received: " << res << "\n";
    });
  });

  std::thread thread_send([&]() {
    protobuf::async_send<com::Message>("127.0.0.1", "25555", message,
                                       [&](std::size_t bytes) {

      std::cout << "bytes sent: " << bytes << "\n";
    });
  });

  thread_send.join();
  thread_receive.join();
}
