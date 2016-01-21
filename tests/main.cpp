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
      REQUIRE_NOTHROW(session.is_socket_unused());
      REQUIRE_NOTHROW(not session.is_ready_for_writting());
      REQUIRE_NOTHROW(not session.is_ready_for_reading());
      REQUIRE_NOTHROW(not session.is_option_activated("state"));
      REQUIRE_NOTHROW(not session.is_option_activated("deadline"));
      REQUIRE_NOTHROW(not session.is_option_activated("heartbeat"));
      REQUIRE_NOTHROW(session.get_heartbeat_message() == "<3");
    }
  }

  GIVEN("testing session options") {
    asio::io_context io_service;
    asio::ip::tcp::socket socket(io_service);
    Hermes::Session<asio::ip::tcp::socket> session(socket);

    WHEN("settting state READING to socket") {
      REQUIRE_NOTHROW(session.set_state_to_socket(Hermes::READING));
      REQUIRE_NOTHROW(not session.is_socket_unused());
      REQUIRE_NOTHROW(not session.is_ready_for_writting());
      REQUIRE_NOTHROW(session.is_ready_for_reading());
    }

    WHEN("setting state WRITTING to socket") {
      REQUIRE_NOTHROW(session.set_state_to_socket(Hermes::WRITTING));
      REQUIRE_NOTHROW(not session.is_socket_unused());
      REQUIRE_NOTHROW(session.is_ready_for_writting());
      REQUIRE_NOTHROW(not session.is_ready_for_reading());
    }

    WHEN("activating options 'state'") {
      REQUIRE_NOTHROW(session.activate_option("state"));
      REQUIRE_NOTHROW(session.is_option_activated("state"));
      REQUIRE_NOTHROW(not session.is_option_activated("deadline"));
      REQUIRE_NOTHROW(not session.is_option_activated("heartbeat"));
    }

    WHEN("activating options 'deadline'") {
      REQUIRE_NOTHROW(session.activate_option("deadline"));
      REQUIRE_NOTHROW(session.is_option_activated("state"));
      REQUIRE_NOTHROW(session.is_option_activated("deadline"));
      REQUIRE_NOTHROW(not session.is_option_activated("heartbeat"));
    }

    WHEN("activating heartbeat option") {
      REQUIRE_NOTHROW(session.activate_option("heartbeat"));
      REQUIRE_NOTHROW(session.is_option_activated("state"));
      REQUIRE_NOTHROW(session.is_option_activated("deadline"));
      REQUIRE_NOTHROW(session.is_option_activated("heartbeat"));
    }

    WHEN("setting new heartbeat message") {
      REQUIRE_NOTHROW(session.set_heartbeat_message("test"));
      REQUIRE_NOTHROW(session.get_heartbeat_message() == "test");
    }
  }
}

SCENARIO("Module Messenger: stream", "[stream]") {
  GIVEN("testing default stream options") {
    asio::io_context io_service;

    auto stream = Hermes::Stream<asio::ip::tcp::socket>::create(io_service);

    WHEN("default") {
      REQUIRE_NOTHROW(stream);
      REQUIRE_NOTHROW(not stream->socket().is_open());
      REQUIRE_NOTHROW(stream->session().is_socket_unused());
      REQUIRE_NOTHROW(not stream->session().is_ready_for_writting());
      REQUIRE_NOTHROW(not stream->session().is_ready_for_reading());
      REQUIRE_NOTHROW(not stream->session().is_option_activated("state"));
      REQUIRE_NOTHROW(not stream->session().is_option_activated("deadline"));
      REQUIRE_NOTHROW(not stream->session().is_option_activated("heartbeat"));
      REQUIRE_NOTHROW(stream->session().get_heartbeat_message() == "<3");
    }
  }

  GIVEN("testing session options") {
    asio::io_context io_service;

    auto stream = Hermes::Stream<asio::ip::tcp::socket>::create(io_service);

    WHEN("settting state READING to socket") {
      REQUIRE_NOTHROW(stream->session().set_state_to_socket(Hermes::READING));
      REQUIRE_NOTHROW(not stream->session().is_socket_unused());
      REQUIRE_NOTHROW(not stream->session().is_ready_for_writting());
      REQUIRE_NOTHROW(stream->session().is_ready_for_reading());
    }

    WHEN("setting state WRITTING to socket") {
      REQUIRE_NOTHROW(stream->session().set_state_to_socket(Hermes::WRITTING));
      REQUIRE_NOTHROW(not stream->session().is_socket_unused());
      REQUIRE_NOTHROW(stream->session().is_ready_for_writting());
      REQUIRE_NOTHROW(not stream->session().is_ready_for_reading());
    }

    WHEN("activating options 'state'") {
      REQUIRE_NOTHROW(stream->session().activate_option("state"));
      REQUIRE_NOTHROW(stream->session().is_option_activated("state"));
      REQUIRE_NOTHROW(not stream->session().is_option_activated("deadline"));
      REQUIRE_NOTHROW(not stream->session().is_option_activated("heartbeat"));
    }

    WHEN("activating options 'deadline'") {
      REQUIRE_NOTHROW(stream->session().activate_option("deadline"));
      REQUIRE_NOTHROW(stream->session().is_option_activated("state"));
      REQUIRE_NOTHROW(stream->session().is_option_activated("deadline"));
      REQUIRE_NOTHROW(not stream->session().is_option_activated("heartbeat"));
    }

    WHEN("activating heartbeat option") {
      REQUIRE_NOTHROW(stream->session().activate_option("heartbeat"));
      REQUIRE_NOTHROW(stream->session().is_option_activated("state"));
      REQUIRE_NOTHROW(stream->session().is_option_activated("deadline"));
      REQUIRE_NOTHROW(stream->session().is_option_activated("heartbeat"));
    }

    WHEN("setting new heartbeat message") {
      REQUIRE_NOTHROW(stream->session().set_heartbeat_message("test"));
      REQUIRE_NOTHROW(stream->session().get_heartbeat_message() == "test");
    }

    WHEN("opening socket") {
      REQUIRE_NOTHROW(stream->socket().open(asio::ip::tcp::v4()));
      REQUIRE_NOTHROW(stream->socket().is_open());
      REQUIRE_NOTHROW(stream->stop());
      REQUIRE_NOTHROW(not stream->socket().is_open());
    }
  }
}

SCENARIO("Module Messenger: TCP", "[software]") {
  GIVEN("Synchronous client connection to a server which is not listenning") {
    auto client = new Hermes::Messenger("client", "tcp", false, "2222");

    WHEN("you try to connect") { REQUIRE_THROWS(client->run()); }
  }

  GIVEN("Synchronous client connection to available server") {
    auto client = new Hermes::Messenger("client", "tcp", false, "2222");
    auto server = new Hermes::Messenger("server", "tcp", true, "2222");

    server->set_connection_handler([&]() { server->disconnect(); });
    WHEN("you try to connect") {
      REQUIRE_NOTHROW(client);
      REQUIRE_NOTHROW(server);
      REQUIRE_NOTHROW(client->run());
      REQUIRE_NOTHROW(client->disconnect());
      REQUIRE_NOTHROW(server->run());
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
        REQUIRE_NOTHROW(test.name() == "name");
        REQUIRE_NOTHROW(test.object() == "object");
        REQUIRE_NOTHROW(test.from() == "from");
        REQUIRE_NOTHROW(test.to() == "to");
        REQUIRE_NOTHROW(test.msg() == "msg");
      });

      std::thread thread_send([&]() {
        std::string serialized;

        message.SerializeToString(&serialized);
        std::this_thread::sleep_for(std::chrono::microseconds(200));
        auto size =
            Hermes::protobuf::send<com::Message>("127.0.0.1", "8247", message);
        REQUIRE_NOTHROW(size == serialized.size());
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
              REQUIRE_NOTHROW(response.name() == "name: ok");
              REQUIRE_NOTHROW(response.object() == "object: ok");
              REQUIRE_NOTHROW(response.from() == "from: ok");
              REQUIRE_NOTHROW(response.to() == "to: ok");
              REQUIRE_NOTHROW(response.msg() == "msg: ok");
            });
      });

      std::thread thread_send([&]() {
        std::this_thread::sleep_for(std::chrono::microseconds(200));
        Hermes::protobuf::async_send<com::Message>(
            "127.0.0.1", "8246", message,
            [](std::size_t bytes) { REQUIRE_NOTHROW(bytes == 49); });
      });

      thread_send.join();
      thread_receive.join();
    }
  }
}
