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


//
// testing Session object
//
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

//
// testing stream's session
//
SCENARIO(
    "Stream class represents a TCP connection. Stream manages operations "
    "on the socket. Furthermore, options can be set to the socket. Stream class "
    "owns a session object, managing socket's option.",
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

//
// testing datagram's session
//
SCENARIO(
    "Datagram class is a handler to manage UDP operations on a socket "
    "Same as Stream, Datagram owns a session object to handle socket's option.",
    "[datagram]") {
  GIVEN("UDP datagram") {
    asio::io_context io_service;

    auto datagram = Hermes::Stream<asio::ip::udp::socket>::create(io_service);

    WHEN("when you check session options") {
      REQUIRE(datagram);
      REQUIRE(not datagram->socket().is_open());
      REQUIRE(datagram->session().is_socket_unused());
      REQUIRE(not datagram->session().is_ready_for_writting());
      REQUIRE(not datagram->session().is_ready_for_reading());
      REQUIRE(not datagram->session().is_option_activated("state"));
      REQUIRE(not datagram->session().is_option_activated("deadline"));
      REQUIRE(not datagram->session().is_option_activated("heartbeat"));
      REQUIRE(datagram->session().get_heartbeat_message() == "<3");
    }
  }

  GIVEN("UDP datagram") {
    asio::io_context io_service;

    auto datagram = Hermes::Stream<asio::ip::udp::socket>::create(io_service);

    WHEN("You set state READING to socket.") {
      datagram->session().set_state_to_socket(Hermes::READING);
      REQUIRE(not datagram->session().is_socket_unused());
      REQUIRE(not datagram->session().is_ready_for_writting());
      REQUIRE(datagram->session().is_ready_for_reading());
    }

    WHEN("You set state WRITTING to socket.") {
      datagram->session().set_state_to_socket(Hermes::WRITTING);
      REQUIRE(not datagram->session().is_socket_unused());
      REQUIRE(datagram->session().is_ready_for_writting());
      REQUIRE(not datagram->session().is_ready_for_reading());
    }

    WHEN("You set a new heartbeat message.") {
      datagram->session().set_heartbeat_message("test");
      REQUIRE_NOTHROW(datagram->session().get_heartbeat_message() == "test");
    }

    WHEN("You open the socket.") {
      datagram->socket().open(asio::ip::udp::v4());
      REQUIRE(datagram->socket().is_open());
    }
  }
}

//
// testing Synchronous tcp softwares
//
SCENARIO("Hermes is able to create Messengers", "[Messenger]") {
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
        "and send to it a message at the connection using the connection "
        "handler.") {
      std::thread a([&]() {
        server->run();
        auto response = server->receive();
        REQUIRE(response == "123456789");
        server->disconnect();
      });

      std::thread b([&]() {
        std::this_thread::sleep_for(std::chrono::microseconds(250));
        client->set_connection_handler([&]() {
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
        "and the server send you a message at the connection using the "
        "connection handler.") {
      std::thread a([&]() {
        server->set_connection_handler([&]() {
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

        auto f = []() {
          bool test = false;

          REQUIRE(not test);
        };

        server->set_disconnection_handler(f);
        server->run();
        server->disconnect();
      });

      std::thread b([&]() {
        std::this_thread::sleep_for(std::chrono::microseconds(250));

        auto f = []() {
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


//
// testing asynchronous tcp softwares
//
SCENARIO(
  "Hermes is able to create Messengers able to perform asynchronous operations",
  "[messenger]") {
  GIVEN("Asynchronous TCP client/server") {
    auto server = new Hermes::Messenger("serVeR", "tcp", true, "8888");
    auto client = new Hermes::Messenger("CliEnT", "tcp", true, "8888");

    WHEN("you connect a client to a server running in a separate thread "
         "and asynchronously send/receive message from server.") {

      std::thread a([&](){

        server->set_connection_handler([&server = server]() {
          auto response = server->receive();
          REQUIRE(response == "test ?");
          server->send("test ok");
          server->disconnect();
        });
        server->run();
      });

      std::thread b([&](){
        std::this_thread::sleep_for(std::chrono::microseconds(250));
        client->set_connection_handler([&](){
          client->async_send("test ?", [](std::size_t bytes) { REQUIRE(bytes == 6); });
          client->async_receive([](std::string msg) { REQUIRE(msg == "test ok"); });
          client->disconnect();
        });
        client->run();
      });


      a.join();
      b.join();
    }
  }

  GIVEN("Asynchronous TCP client/server") {
    auto server = new Hermes::Messenger("serVeR", "tcp", true, "8888");

    WHEN("you connect then disconnect 100 clients to a server running in a separate thread.") {

      std::thread a([&](){

        int i = 0;

        server->set_connection_handler([&]() {
          std::cout << "connection n° " << i++ << std::endl;
          if (i == 100)
            server->disconnect();
        });
        server->run();
      });

      std::thread b([&](){
        for (int i = 0; i < 100; i++) {
          auto client = new Hermes::Messenger("client", "tcp", true, "8888");

          std::this_thread::sleep_for(std::chrono::microseconds(250));
          client->set_connection_handler([&](){
            std::cout << "client n° " << i << " connected :)" << std::endl;
            client->disconnect();
          });
          client->run();
        }
      });

      a.join();
      b.join();
    }
  }

  GIVEN("Asynchronous TCP client/server") {
    auto server = new Hermes::Messenger("serVeR", "tcp", true, "8888");

    WHEN("you do 100 times: connect a client send/receive a message to a server running "
         "in a separate thread.") {

      std::thread a([&](){

        int i = 0;

        server->set_connection_handler([&]() {
          i++;
          auto response = server->receive();
          REQUIRE(response == "(.)(.)");
          server->send("<3");
          if (i == 100)
            server->disconnect();
        });
        server->run();
      });

      std::thread b([&](){
        for (int i = 0; i < 100; i++) {
          auto client = new Hermes::Messenger("client", "tcp", true, "8888");

          std::this_thread::sleep_for(std::chrono::microseconds(250));
          client->set_connection_handler([&](){
            client->async_send("(.)(.)", [](std::size_t b) { REQUIRE(b == 6); });
            client->async_receive([](std::string msg) { REQUIRE(msg == "<3"); });
            client->disconnect();
          });
          client->run();
        }
      });

      a.join();
      b.join();
    }
  }
}


//
// testing synchronous udp softwares
//
// SCENARIO() {
//   GIVEN("Synchronous UDP client/server") {
//     auto server = new Hermes::Messenger("serVeR", "UDP", false, "8888");
//     auto client = new Hermes::Messenger("CliEnT", "udP", false, "8888");
//
//     WHEN("you send a message") {
//
//       std::thread a([&](){
//         server->run();
//         auto response = server->receive();
//         std::cout << response << std::endl;
//         server->disconnect();
//       });
//
//       std::thread b([&](){
//           std::this_thread::sleep_for(std::chrono::microseconds(250));
//           client->run();
//           client->send("123456789\n");
//           client->send("987654321\n");
//           // auto response = client->receive();
//           // std::cout << response << std::endl;
//           client->disconnect();
//       });
//
//
//       a.join();
//       b.join();
//     }
//   }
// }




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
                std::cout << ":)\n";
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
