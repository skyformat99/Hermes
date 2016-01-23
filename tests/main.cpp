#define CATCH_CONFIG_MAIN
#include "catch.hpp"
#include "Hermes.hpp"

#include "Communication.pb.h"

SCENARIO("Module Messenger: session", "[session]") {
  GIVEN("testing default session options") {
    asio::io_context io_service;
    asio::ip::tcp::socket socket(io_service);
    Hermes::Session<asio::ip::tcp::socket> session(socket);

    WHEN("default") {
      REQUIRE(session.is_socket_unused() == true);
      REQUIRE(not session.is_ready_for_writting());
      REQUIRE(not session.is_ready_for_reading());
      REQUIRE(not session.is_option_activated("state"));
      REQUIRE(not session.is_option_activated("deadline"));
      REQUIRE(not session.is_option_activated("heartbeat"));
      REQUIRE(session.get_heartbeat_message() == "<3");
    }
  }

  GIVEN("testing session options") {
    asio::io_context io_service;
    asio::ip::tcp::socket socket(io_service);
    Hermes::Session<asio::ip::tcp::socket> session(socket);

    WHEN("settting state READING to socket") {
      session.set_state_to_socket(Hermes::READING);
      REQUIRE(not session.is_socket_unused());
      REQUIRE(not session.is_ready_for_writting());
      REQUIRE(session.is_ready_for_reading());
    }

    WHEN("setting state WRITTING to socket") {
      session.set_state_to_socket(Hermes::WRITTING);
      REQUIRE(not session.is_socket_unused());
      REQUIRE(session.is_ready_for_writting());
      REQUIRE(not session.is_ready_for_reading());
    }

    WHEN("setting new heartbeat message") {
      session.set_heartbeat_message("test");
      REQUIRE(session.get_heartbeat_message() == "test");
    }
  }
}

SCENARIO("Module Messenger: stream", "[stream]") {
  GIVEN("testing default stream options") {
    asio::io_context io_service;

    auto stream = Hermes::Stream<asio::ip::tcp::socket>::create(io_service);

    WHEN("default") {
      REQUIRE(stream);
      REQUIRE(not stream->socket().is_open());
      REQUIRE(stream->session().is_socket_unused());
      REQUIRE(not stream->session().is_ready_for_writting());
      REQUIRE(not stream->session().is_ready_for_reading());
      REQUIRE(not stream->session().is_option_activated("state"));
      REQUIRE(not stream->session().is_option_activated("deadline"));
      REQUIRE(not stream->session().is_option_activated("heartbeat"));
      REQUIRE(stream->session().get_heartbeat_message() == "<3");
    }
  }

  GIVEN("testing session options") {
    asio::io_context io_service;

    auto stream = Hermes::Stream<asio::ip::tcp::socket>::create(io_service);

    WHEN("settting state READING to socket") {
      stream->session().set_state_to_socket(Hermes::READING);
      REQUIRE(not stream->session().is_socket_unused());
      REQUIRE(not stream->session().is_ready_for_writting());
      REQUIRE(stream->session().is_ready_for_reading());
    }

    WHEN("setting state WRITTING to socket") {
      stream->session().set_state_to_socket(Hermes::WRITTING);
      REQUIRE(not stream->session().is_socket_unused());
      REQUIRE(stream->session().is_ready_for_writting());
      REQUIRE(not stream->session().is_ready_for_reading());
    }

    WHEN("setting new heartbeat message") {
      stream->session().set_heartbeat_message("test");
      REQUIRE_NOTHROW(stream->session().get_heartbeat_message() == "test");
    }

    WHEN("opening socket") {
      stream->socket().open(asio::ip::tcp::v4());
      REQUIRE(stream->socket().is_open());
      REQUIRE_NOTHROW(stream->stop());
      REQUIRE(not stream->socket().is_open());
    }
  }
}

SCENARIO("Module serialization", "[protobuf]") {
  GIVEN("testing Hermes synchronous protobuf operations.") {
    com::Message message;

    message.set_name("name");
    message.set_object("object");
    message.set_from("from");
    message.set_to("to");
    message.set_msg("msg");

    WHEN("not async") {
      std::thread thread_receive([]() {
        auto test = Hermes::protobuf::receive<com::Message>("8247");
        REQUIRE(test.name() == "name");
        REQUIRE(test.object() == "object");
        REQUIRE(test.from() == "from");
        REQUIRE(test.to() == "to");
        REQUIRE(test.msg() == "msg");
      });

      std::thread thread_send([&]() {
        std::string serialized;

        message.SerializeToString(&serialized);
        std::this_thread::sleep_for(std::chrono::microseconds(200));
        auto size =
            Hermes::protobuf::send<com::Message>("127.0.0.1", "8247", message);
        REQUIRE(size == serialized.size());
      });

      thread_send.join();
      thread_receive.join();
    }
  }

  GIVEN("testing Hermes asynchronous protobuf operations.") {
    com::Message message;

    message.set_name("name: ok");
    message.set_object("object: ok");
    message.set_from("from: ok");
    message.set_to("to: ok");
    message.set_msg("msg: ok");

    WHEN("async") {
      std::thread thread_receive([&]() {
        Hermes::protobuf::async_receive<com::Message>(
            "8246", [](com::Message response) {
              REQUIRE(response.name() == "name: ok");
              REQUIRE(response.object() == "object: ok");
              REQUIRE(response.from() == "from: ok");
              REQUIRE(response.to() == "to: ok");
              REQUIRE(response.msg() == "msg: ok");
            });
      });

      std::thread thread_send([&]() {
        std::this_thread::sleep_for(std::chrono::microseconds(200));
        Hermes::protobuf::async_send<com::Message>(
            "127.0.0.1", "8246", message,
            [](std::size_t bytes) { REQUIRE(bytes == 49); });
      });

      thread_send.join();
      thread_receive.join();
    }
  }
}
