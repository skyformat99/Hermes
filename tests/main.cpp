#define CATCH_CONFIG_MAIN
#include "catch.hpp"
#include "Hermes.hpp"

#include "Communication.pb.h"

/**
*
* Module Messenger: tests
*
*
*/

SCENARIO("Session object, a socket options manager.", "[Messenger::session]") {
  GIVEN("Session class") {
    asio::io_context io_service;
    asio::ip::tcp::socket socket(io_service);
    Hermes::Session<asio::ip::tcp::socket> session(socket);

    WHEN("you check options") {
      REQUIRE(session.is_socket_unused() == true);
      REQUIRE(not session.is_ready_for_writting());
      REQUIRE(not session.is_ready_for_reading());
      REQUIRE(not session.is_option_activated("state"));
      REQUIRE(not session.is_option_activated("deadline"));
      REQUIRE(not session.is_option_activated("heartbeat"));
      REQUIRE(session.get_heartbeat_message() == "<3");
    }
  }

  GIVEN("Session class") {
    asio::io_context io_service;
    asio::ip::tcp::socket socket(io_service);
    Hermes::Session<asio::ip::tcp::socket> session(socket);

    WHEN("you set state READING to socket") {
      session.set_state_to_socket(Hermes::READING);
      REQUIRE(not session.is_socket_unused());
      REQUIRE(not session.is_ready_for_writting());
      REQUIRE(session.is_ready_for_reading());
    }

    WHEN("you set state WRITTING to socket") {
      session.set_state_to_socket(Hermes::WRITTING);
      REQUIRE(not session.is_socket_unused());
      REQUIRE(session.is_ready_for_writting());
      REQUIRE(not session.is_ready_for_reading());
    }

    WHEN("you set new heartbeat message") {
      session.set_heartbeat_message("test");
      REQUIRE(session.get_heartbeat_message() == "test");
    }
  }
}

SCENARIO(
    "Messenger owns a stream. This object handles operations on a socket. "
    "To manage those operations the object stream owns a session object"
    "which is responsible of socket's options",
    "[stream]") {
  GIVEN("TCP stream") {
    asio::io_context io_service;

    auto stream = Hermes::Stream<asio::ip::tcp::socket>::create(io_service);

    WHEN("when you check session options") {
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

  GIVEN("TCP stream") {
    asio::io_context io_service;

    auto stream = Hermes::Stream<asio::ip::tcp::socket>::create(io_service);

    WHEN("You set state READING to socket.") {
      stream->session().set_state_to_socket(Hermes::READING);
      REQUIRE(not stream->session().is_socket_unused());
      REQUIRE(not stream->session().is_ready_for_writting());
      REQUIRE(stream->session().is_ready_for_reading());
    }

    WHEN("You set state WRITTING to socket.") {
      stream->session().set_state_to_socket(Hermes::WRITTING);
      REQUIRE(not stream->session().is_socket_unused());
      REQUIRE(stream->session().is_ready_for_writting());
      REQUIRE(not stream->session().is_ready_for_reading());
    }

    WHEN("You set a new heartbeat message.") {
      stream->session().set_heartbeat_message("test");
      REQUIRE_NOTHROW(stream->session().get_heartbeat_message() == "test");
    }

    WHEN("You open the socket.") {
      stream->socket().open(asio::ip::tcp::v4());
      REQUIRE(stream->socket().is_open());
    }
  }
}

SCENARIO("Hermes is able to create Messengers (network software)",
         "[Messenger]") {
  GIVEN("Synchronous TCP server/client") {
    auto server = new Hermes::Messenger("server", "tcp", false, "8888");
    auto client = new Hermes::Messenger("CliEnT", "TcP", false, "8888");

    WHEN(
        "you connect a client to a server running in a separate thread "
        "and you send a message to the server.") {
      std::thread a([&]() {
        server->run();
        auto response = server->receive();
        REQUIRE(response == "123456789");
        server->disconnect();
      });

      std::thread b([&]() {
        std::this_thread::sleep_for(std::chrono::microseconds(250));
        client->run();
        client->send("123456789");
        client->disconnect();
      });

      a.join();
      b.join();
    }

    WHEN(
        "you connect a client to a server running in a separate thread "
        "and the server send you a message at the connection") {
      std::thread a([&]() {
        server->run();
        server->send("987654321");
        server->disconnect();
      });

      std::thread b([&]() {
        std::this_thread::sleep_for(std::chrono::microseconds(250));
        client->run();
        auto response = client->receive();
        REQUIRE(response == "987654321");
        client->disconnect();
      });

      a.join();
      b.join();
    }

    WHEN(
        "you connect a client to a server running in a separate thread "
        "and send to it a message at the connection using the connection handler.") {
      std::thread a([&]() {
        server->run();
        auto response = server->receive();
        REQUIRE(response == "123456789");
        server->disconnect();
      });

      std::thread b([&]() {
        std::this_thread::sleep_for(std::chrono::microseconds(250));
        client->set_connection_handler([&](){
        client->send("123456789");
        client->disconnect();
        });
        client->run();
      });

      a.join();
      b.join();
    }

    WHEN(
        "you connect a client to a server running in a separate thread "
        "and the server send you a message at the connection using the connection handler.") {
      std::thread a([&]() {
        server->set_connection_handler([&](){
          server->send("987654321");
          server->disconnect();
        });
        server->run();
      });

      std::thread b([&]() {
        std::this_thread::sleep_for(std::chrono::microseconds(250));
        client->run();
        auto response = client->receive();
        REQUIRE(response == "987654321");
        client->disconnect();
      });

      a.join();
      b.join();
    }

    WHEN(
        "you connect a client to a server running in a separate thread "
        "and you use the disconnect handlers.") {
      std::thread a([&]() {

        auto f = [](){
          bool test = false;

          REQUIRE(not test);
        };

        server->set_disconnection_handler(f);
        server->run();
        server->disconnect();
      });

      std::thread b([&]() {
        std::this_thread::sleep_for(std::chrono::microseconds(250));

        auto f = [](){
          bool test = true;

          REQUIRE(test);
        };

        client->set_disconnection_handler(f);
        client->run();
        client->disconnect();
      });

      a.join();
      b.join();
    }
  }
}

/**
*
*
* Module Serialization - protobuf: tests
*
*
*/
SCENARIO("Hermes is able to send and receive serialized data.", "[protobuf]") {
  GIVEN("Google protobuf message") {
    com::Message message;

    message.set_name("name");
    message.set_object("object");
    message.set_from("from");
    message.set_to("to");
    message.set_msg("msg");

    WHEN(
        "you send a serialized protobuf message from a thread "
        "and you receive and unserialize it from another thread") {
      std::thread thread_receive([]() {
        auto test = Hermes::protobuf::receive<com::Message>("8888");
        REQUIRE(test.name() == "name");
        REQUIRE(test.object() == "object");
        REQUIRE(test.from() == "from");
        REQUIRE(test.to() == "to");
        REQUIRE(test.msg() == "msg");
      });

      std::thread thread_send([&]() {
        std::string serialized;

        message.SerializeToString(&serialized);
        std::this_thread::sleep_for(std::chrono::microseconds(150));
        auto size =
            Hermes::protobuf::send<com::Message>("127.0.0.1", "8888", message);
        REQUIRE(size == serialized.size());
      });

      thread_send.join();
      thread_receive.join();
    }

    WHEN(
        "you send 100 serialized protobuf messages from a thread "
        "and you receive and unserialize them from another thread") {
      std::thread a([&]() {
        for (int i = 0; i < 100; i++) {
          auto test = Hermes::protobuf::receive<com::Message>("8888");
          REQUIRE(test.name() == "name");
          REQUIRE(test.object() == "object");
          REQUIRE(test.from() == "from");
          REQUIRE(test.to() == "to");
          REQUIRE(test.msg() == "msg");
        }
      });

      std::thread b([&]() {
        for (int i = 0; i < 100; i++) {
          std::string serialized;

          message.SerializeToString(&serialized);
          std::this_thread::sleep_for(std::chrono::microseconds(150));
          auto size = Hermes::protobuf::send<com::Message>("127.0.0.1", "8888",
                                                           message);
          REQUIRE(size == serialized.size());
        }
      });

      a.join();
      b.join();
    }
  }

  GIVEN("Google protobuf message") {
    com::Message message;

    message.set_name("name: ok");
    message.set_object("object: ok");
    message.set_from("from: ok");
    message.set_to("to: ok");
    message.set_msg("msg: ok");

    WHEN(
        "you send, asynchronously, a serialized protobuf message from a thread "
        "and you receive and unserialize it from another thread") {
      std::thread thread_receive([&]() {
        Hermes::protobuf::async_receive<com::Message>(
            "8888", [](com::Message response) {
              REQUIRE(response.name() == "name: ok");
              REQUIRE(response.object() == "object: ok");
              REQUIRE(response.from() == "from: ok");
              REQUIRE(response.to() == "to: ok");
              REQUIRE(response.msg() == "msg: ok");
            });
      });

      std::thread thread_send([&]() {
        std::this_thread::sleep_for(std::chrono::microseconds(150));
        Hermes::protobuf::async_send<com::Message>(
            "127.0.0.1", "8888", message,
            [](std::size_t bytes) { REQUIRE(bytes == 49); });
      });

      thread_send.join();
      thread_receive.join();
    }

    WHEN(
        "you send, asynchronously, 100 serialized protobuf messages from a "
        "thread "
        "and you receive and unserialize it from another thread") {
      std::thread a([&]() {
        for (int i = 0; i < 100; i++) {
          Hermes::protobuf::async_receive<com::Message>(
              "8888", [](com::Message response) {
                REQUIRE(response.name() == "name: ok");
                REQUIRE(response.object() == "object: ok");
                REQUIRE(response.from() == "from: ok");
                REQUIRE(response.to() == "to: ok");
                REQUIRE(response.msg() == "msg: ok");
              });
        }
      });

      std::thread b([&]() {
        for (int i = 0; i < 100; i++) {
          std::this_thread::sleep_for(std::chrono::microseconds(250));
          Hermes::protobuf::async_send<com::Message>(
              "127.0.0.1", "8888", message,
              [](std::size_t bytes) { REQUIRE(bytes == 49); });
        }
      });

      a.join();
      b.join();
    }
  }
}
