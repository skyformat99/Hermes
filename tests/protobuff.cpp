#include "Hermes.hpp"
#include "Communication.pb.h"

#include <thread>
#include <iostream>
#include <unistd.h>
#include <assert.h>

using namespace Hermes;

void test_protobuf_synchronous_operations() {
  std::cout << "[Protobuf] testing synchronous operations." << std::endl;
  com::Message message;

  message.set_name("name");
  message.set_object("object");
  message.set_from("from");
  message.set_to("to");
  message.set_msg("msg");

  std::thread thread_receive([](){
      auto test = protobuf::receive<com::Message>("8247");
      assert(test.name() == "name");
      assert(test.object() == "object");
      assert(test.from() == "from");
      assert(test.to() == "to");
      assert(test.msg() == "msg");
      std::cout << "-> test synchronous operations [ok]." << std::endl;
  });

  std::thread thread_send([&](){
      std::string serialized;

      message.SerializeToString(&serialized);
      std::this_thread::sleep_for(std::chrono::microseconds(200));
      auto size = protobuf::send<com::Message>("127.0.0.1", "8247", message);
      assert(size == serialized.size());
  });

  thread_send.join();
  thread_receive.join();
}

void test_protobuf_asynchronous_operations() {
  std::cout << "[Protobuf] testing asynchronous operations." << std::endl;
  com::Message message;
  com::Message response;

  message.set_name("name: ok");
  message.set_object("object: ok");
  message.set_from("from: ok");
  message.set_to("to: ok");
  message.set_msg("msg: ok");

  std::thread thread_receive([&]() {
    protobuf::async_receive<com::Message>("8246", [](com::Message response) {
      assert(response.name() == "name: ok");
      assert(response.object() == "object: ok");
      assert(response.from() == "from: ok");
      assert(response.to() == "to: ok");
      assert(response.msg() == "msg: ok");
      std::cout << "-> test asynchronous operations [ok]." << std::endl;
    });
  });

  std::thread thread_send([&]() {
      protobuf::async_send<com::Message>("127.0.0.1", "8246", message, [](std::size_t bytes){
        assert(bytes == 49);
      });
    });

  thread_send.join();
  thread_receive.join();
}
