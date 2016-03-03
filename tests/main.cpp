#define CATCH_CONFIG_MAIN
#include "catch.hpp"
#include "Hermes.hpp"

#include "Communication.pb.h"

using namespace hermes::core;
using namespace hermes::network;

SCENARIO("I/O Services", "[core]") {
  GIVEN("Service object, routine function, f function to print parameter") {
    Service service;
    bool test = true;

    auto routine = [](asio::io_context& service) { service.run(); };

    auto f = [](size_t n) {

      std::this_thread::sleep_for(std::chrono::microseconds(1000));

      {
        std::mutex mutex;
        std::lock_guard<std::mutex> lock(mutex);
        std::cout << "[" << std::this_thread::get_id() << "] " << n
                  << std::endl;
      }

    };

    WHEN(
        "Running the service in a dedicated thread without adding a work."
        "\n>>> should not be stuck") {
      service.run();
      REQUIRE(test);
      service.stop();
    }

    WHEN(
        "Reset work object to run the service in our main thread."
        "\n>>> should not be stuck") {
      test = false;
      service.get_work().reset();
      service.get().run();
      REQUIRE(not test);
      service.get().stop();
    }

    WHEN(
        "Testing post() method using a thread pool."
        "\n>>> no error should be thrown") {
      std::vector<std::thread> threads;

      for (int i = 0; i < 2; ++i) {
        threads.push_back(
            std::thread(std::bind(routine, std::ref(service.get()))));
      }

      service.post(std::bind(f, 1));
      service.post(std::bind(f, 2));
      service.post(std::bind(f, 3));
      service.post(std::bind(f, 4));

      service.get_work().reset();
      for (int i = 0; i < 2; i++) REQUIRE_NOTHROW(threads[i].join());
    }

    WHEN(
        "Testing strand object."
        "\n>>> no error should be thrown") {
      std::vector<std::thread> threads;

      for (int i = 0; i < 2; ++i) {
        threads.push_back(
            std::thread(std::bind(routine, std::ref(service.get()))));
      }

      service.get_strand().post(std::bind(f, 5));
      service.get_strand().post(std::bind(f, 6));
      service.get_strand().post(std::bind(f, 7));
      service.get_strand().post(std::bind(f, 8));

      service.get_work().reset();
      for (int i = 0; i < 2; i++) REQUIRE_NOTHROW(threads[i].join());
    }

    WHEN("testing stop() service") {
      REQUIRE(not service.is_stop());
      service.stop();
      REQUIRE(service.is_stop());
    }
  }
}

SCENARIO("Dedicated class for Error handling", "[core]") {
  GIVEN("Error class, 4 object functions to test the different error type") {
    Error error;

    auto user = []() { throw Error::User("logic error"); };

    auto connection =
        []() { throw Error::Connection("connect operation failed"); };

    auto write = []() { throw Error::Write("write operation failed"); };

    auto read = []() { throw Error::Read("Read operation failed"); };

    WHEN("throwing logic error made by the user") {
      try {
        user();
      } catch (std::exception& e) {
        REQUIRE(std::string(e.what()) == "logic error");
      }
    }

    WHEN("throwing connect exception") {
      try {
        connection();
      } catch (std::exception& e) {
        REQUIRE(std::string(e.what()) == "connect operation failed");
      }
    }

    WHEN("throwing write exception") {
      try {
        write();
      } catch (std::exception& e) {
        REQUIRE(std::string(e.what()) == "write operation failed");
      }
    }

    WHEN("throwing read exception") {
      try {
        read();
      } catch (std::exception& e) {
        REQUIRE(std::string(e.what()) == "Read operation failed");
      }
    }
  }
}

SCENARIO("testing Stream session features and thread safety", "[network]") {
  GIVEN("I/O service object") {
    Service service;

    WHEN(
        "creating a new stream session."
        "\n>>> should not be stuck") {
      auto session = Stream::new_session(service);

      session->service().get_work().reset();
      session->service().get().run();
      REQUIRE(session);
      REQUIRE(not session->service().is_stop());
      REQUIRE(not session->socket().is_open());
      session->service().get().stop();
    }

    WHEN(
        "adding a work to the session."
        "\n>>> should print [thread_id_in_wich_run_service_has_been_called] :)") {
      auto session = Stream::new_session(service);

      auto fct = [](const std::string& debug) {
        std::cout << "[" << std::this_thread::get_id() << "] " << debug << "\n";
        REQUIRE(debug == ":)");
      };

      session->service().run();
      session->service().get_strand().post(std::bind(fct, ":)"));
      session->service().stop();
    }

    WHEN(
        "connecting the tcp socket in local to an used port to throw an error "
        "\nthen to google to success. Disconnecting the session at the end.") {
      auto session = Stream::new_session(service);

      asio::ip::tcp::resolver resolver(service.get());

      // No server listenning on the specified port.
      // connection to endpoint2 has to throw.
      asio::ip::tcp::resolver::query query2("127.0.0.1", "8888");
      asio::ip::tcp::endpoint endpoint2 = *resolver.resolve(query2);
      REQUIRE_THROWS(session->connect(endpoint2));

      asio::ip::tcp::resolver::query query("www.google.com", "80");
      asio::ip::tcp::endpoint endpoint = *resolver.resolve(query);
      REQUIRE_NOTHROW(session->connect(endpoint));

      auto disconnected = false;

      session->service().run();
      session->disconnect();
      disconnected = true;
      session->service().stop();
      REQUIRE(disconnected);
    }

    WHEN(
        "Passing a reference on the stream to many threads to call disconnect()"
        "\nTesting Thread Safety.") {
      auto session = Stream::new_session(service);

      auto fct = [](Stream::session& session) { session->disconnect(); };

      asio::ip::tcp::resolver resolver(service.get());
      asio::ip::tcp::resolver::query query("www.google.com", "80");
      asio::ip::tcp::endpoint endpoint = *resolver.resolve(query);

      session->service().run();
      REQUIRE_NOTHROW(session->connect(endpoint));

      std::vector<std::thread> threads;
      for (int i = 0; i < 100; i++)
        threads.push_back(std::thread(std::bind(fct, session)));

      for (int i = 0; i < 100; i++) threads[i].join();

      session->service().stop();
      REQUIRE(not session->is_connected());
    }

    /**
    *     <<< Tests with ncat >>>
    *
    *
    *
    *   The followings scenarios are commented to ensure that the build wont
    *   fail on travis.
    *   No server will be listenning on the specified port.
    *
    *
    *
    */

    // WHEN("sending tcp message to netcat on port 9999.") {
    //   auto session = Stream::new_session(service);
    //
    //   asio::ip::tcp::resolver resolver(service.get());
    //   asio::ip::tcp::resolver::query query("127.0.0.1", "9999");
    //   asio::ip::tcp::endpoint endpoint = *resolver.resolve(query);
    //   session->service().run();
    //   REQUIRE_NOTHROW(session->connect(endpoint));
    //   session->send("test :)");
    //   auto response = session->receive();
    //   std::cout << response << std::endl;
    //   session->disconnect();
    //   session->service().stop();
    //   REQUIRE(not session->is_connected());
    // }

    // WHEN("sending tcp message to netcat on port 9999 from 100 threads.") {
    //   auto session = Stream::new_session(service);
    //
    //   auto fct = [](Stream::session& session) {
    //     auto bytes = session->send(":)");
    //     REQUIRE(bytes == 2);
    //     auto response = session->receive();
    //     {
    //       std::mutex mutex;
    //       std::lock_guard<std::mutex> lock(mutex);
    //       std::cout << response << std::endl;
    //     }
    //   };
    //
    //   session->service().run();
    //
    //   asio::ip::tcp::resolver resolver(service.get());
    //
    //   asio::ip::tcp::resolver::query query("127.0.0.1", "9999");
    //   asio::ip::tcp::endpoint endpoint = *resolver.resolve(query);
    //   REQUIRE_NOTHROW(session->connect(endpoint));
    //
    //   std::vector<std::thread> threads;
    //   for (int i = 0; i < 20; i++)
    //     threads.push_back(std::thread(std::bind(fct, session)));
    //
    //   for (int i = 0; i < 20; i++) threads[i].join();
    //
    //   session->disconnect();
    //   session->service().stop();
    //   REQUIRE(not session->is_connected());
    // }

    // WHEN("asynchronous connection") {
    //   auto session = Stream::new_session(service);
    //   asio::ip::tcp::resolver resolver(service.get());
    //   asio::ip::tcp::resolver::query query("127.0.0.1", "9999");
    //   asio::ip::tcp::endpoint endpoint = *resolver.resolve(query);
    //
    //   REQUIRE_NOTHROW(session->async_connect(endpoint));
    //   session->disconnect();
    //   session->service().stop();
    //   REQUIRE(not session->is_connected());
    // }

    // WHEN("asynchronous connection and asynchronous send") {
    //   auto session = Stream::new_session(service);
    //
    //   asio::ip::tcp::resolver resolver(service.get());
    //   asio::ip::tcp::resolver::query query("127.0.0.1", "9999");
    //   asio::ip::tcp::endpoint endpoint = *resolver.resolve(query);
    //
    //   REQUIRE_NOTHROW(session->async_connect(endpoint));
    //
    //   session->set_write_handler([](std::size_t bytes, Stream& session){
    //     std::cout << "bytes: " << bytes << std::endl;
    //     session.send("<3<3<3<3");
    //     std::cout << session.receive() << std::endl;
    //   });
    //
    //   session->async_send("123456789\n");
    //   session->send("987645321\n");
    //   std::this_thread::sleep_for(std::chrono::microseconds(1000));
    //   session->disconnect();
    //   session->service().stop();
    //   REQUIRE(not session->is_connected());
    // }

    // WHEN("asynchronous connection + 100 asynchronous send") {
    //   auto session = Stream::new_session(service);
    //
    //   auto fct = [](Stream::session& session){
    //     session->async_send(":)");
    //   };
    //
    //   asio::ip::tcp::resolver resolver(service.get());
    //   asio::ip::tcp::resolver::query query("127.0.0.1", "9999");
    //   asio::ip::tcp::endpoint endpoint = *resolver.resolve(query);
    //
    //   REQUIRE_NOTHROW(session->async_connect(endpoint));
    //
    //   session->set_write_handler([](std::size_t bytes, Stream& session){
    //     std::cout << "bytes: " << bytes << std::endl;
    //     session.send("<3<3<3<3");
    //   });
    //
    //   std::vector<std::thread> threads;
    //   for (int i = 0; i < 100; i++)
    //     threads.push_back(std::thread(std::bind(fct, session)));
    //
    //   for (int i = 0; i < 100; i++) threads[i].join();
    //
    //   std::this_thread::sleep_for(std::chrono::microseconds(5000));
    //   session->disconnect();
    //   session->service().stop();
    //   REQUIRE(not session->is_connected());
    // }

    // WHEN("asynchronous connection/receive") {
    //   auto session = Stream::new_session(service);
    //
    //   asio::ip::tcp::resolver resolver(service.get());
    //   asio::ip::tcp::resolver::query query("127.0.0.1", "9999");
    //   asio::ip::tcp::endpoint endpoint = *resolver.resolve(query);
    //
    //   REQUIRE_NOTHROW(session->async_connect(endpoint));
    //
    //   session->set_read_handler([](std::string response, Stream& session){
    //     std::cout << "response: " << response << std::endl;
    //   });
    //
    //   session->async_receive();
    //   std::cout << session->receive() << std::endl;
    //   std::this_thread::sleep_for(std::chrono::microseconds(10000));
    //   session->disconnect();
    //   session->service().stop();
    //   REQUIRE(not session->is_connected());
    // }

    // WHEN("asynchronous connection/receive + 100 async_receive") {
    //   auto session = Stream::new_session(service);
    //
    //     auto fct = [](Stream::session& session) {
    //       session->async_receive();
    //   };
    //
    //   asio::ip::tcp::resolver resolver(service.get());
    //   asio::ip::tcp::resolver::query query("127.0.0.1", "9999");
    //   asio::ip::tcp::endpoint endpoint = *resolver.resolve(query);
    //
    //   REQUIRE_NOTHROW(session->async_connect(endpoint));
    //
    //   session->set_read_handler([](std::string response, Stream& session){
    //     std::cout << "response: " << response << std::endl;
    //   });
    //
    //   std::vector<std::thread> threads;
    //   for (int i = 0; i < 100; i++)
    //     threads.push_back(std::thread(std::bind(fct, session)));
    //
    //   for (int i = 0; i < 100; i++) threads[i].join();
    //
    //   std::this_thread::sleep_for(std::chrono::microseconds(100000));
    //   session->disconnect();
    //   session->service().stop();
    //   REQUIRE(not session->is_connected());
    // }

  }
}

/**
*     <<< Tests with ncat >>>
*
*
*
*   The followings scenarios are commented to ensure that the build wont
*   fail on travis.
*   No server will be listenning on the specified port.
*
*
*
*/

// SCENARIO("testing tcp client behavior and thread safety", "[tcp]") {
//
//   GIVEN("TCP client") {
//
//     hermes::tcp::Client client("127.0.0.1", "9999");
//
//     WHEN("connecting the client and perform send/receive operation") {
//
//       client.connect();
//       client.send("message");
//       std::cout << client.receive() << std::endl;
//       client.disconnect();
//     }
//
//     WHEN("using asynchronous operations") {
//
//       client.set_send_handler([](std::size_t bytes, hermes::network::Stream& session){
//         std::cout << "bytes: " << bytes << std::endl;
//       });
//
//       client.set_receive_handler([](std::string received, hermes::network::Stream& session){
//         std::cout << "received: " << received << std::endl;
//       });
//
//       client.async_connect();
//       client.async_send("test1");
//       client.send("test2");
//       client.async_receive();
//       std::cout << client.receive() << std::endl;
//       std::this_thread::sleep_for(std::chrono::seconds(1));
//       client.disconnect();
//     }
//
//     WHEN("testing thread safety") {
//
//       auto fct = [](hermes::tcp::Client& client) {
//         client.async_send("async\n");
//         client.send("not async\n");
//       };
//
//       client.set_send_handler([](std::size_t bytes, hermes::network::Stream& session){
//         std::cout << "bytes: " << bytes << std::endl;
//       });
//
//       REQUIRE_NOTHROW(client.async_connect());
//       REQUIRE(client.is_connected());
//
//       std::vector<std::thread> threads;
//
//       for (int i = 0; i < 100; i++)
//         threads.push_back(std::thread(std::bind(fct, std::ref(client))));
//
//       for (int i = 0; i < 100; i++) threads[i].join();
//
//       client.disconnect();
//       REQUIRE(not client.is_connected());
//     }
//   }
// }


// SCENARIO("testing tcp server behavior and thread safety", "[tcp]") {
//   GIVEN("TCP server listenning on port 9999") {
//     hermes::tcp::Server server("9999");
//
//     WHEN("testing iterative server") {
//
//       auto handler = [](Stream::session connection){
//
//         connection->send("test\n");
//         std::cout << connection->receive() << std::endl;
//       };
//
//       server.set_accept_handler(handler);
//
//       for (int i = 0; i < 2; ++i)
//         server.run(false);
//     }
//
//     WHEN("testing asynchronous server") {
//
//       auto handler = [](Stream::session connection){
//
//         auto send = [](std::size_t bytes, Stream& session){
//           std::cout << ">>> " << bytes << std::endl;
//         };
//
//         auto rcv = [](std::string r, Stream& session){
//           std::cout << "<<< " << r << std::endl;
//         };
//
//         connection->set_write_handler(send);
//         connection->set_read_handler(rcv);
//         connection->async_send("test\n");
//         connection->async_receive();
//       };
//
//       server.set_accept_handler(handler);
//
//       server.run(true);
//     }
//   }
// }

SCENARIO("testing hermes protobuf operations", "[protobuf]") {
  GIVEN("protobuf message") {
      com::Message message;

      message.set_name("aaaa");
      message.set_object("bbbb");
      message.set_from("cccc");
      message.set_to("dddd");
      message.set_msg("eeee");

      WHEN("testing send and receive from 2 separate threads") {

        std::thread a([&](){
          std::this_thread::sleep_for(std::chrono::seconds(1));
          REQUIRE_NOTHROW(
                hermes::protobuf::send<com::Message>(
                  "127.0.0.1", "50501", message
          ));
        });

        std::thread b([](){
          std::string handler("");

          auto result = hermes::protobuf::receive<com::Message>("50501");
          REQUIRE(result.object() == "bbbb");
        });


        a.join();
        b.join();
      }


      WHEN("testing async_send") {

        std::thread a([&](){
          std::this_thread::sleep_for(std::chrono::seconds(1));
          REQUIRE_NOTHROW(
                hermes::protobuf::async_send<com::Message>(
                  "127.0.0.1", "50501", message,
                  [](std::size_t bytes) {
                    std::cout << "bytes: " << bytes << std::endl;
                    REQUIRE(bytes == 30);
                  }
          ));
        });

        std::thread b([](){
          auto result = hermes::protobuf::receive<com::Message>("50501");
          REQUIRE(result.object() == "bbbb");
        });


        a.join();
        b.join();
      }


      WHEN("testing async_receive") {

        std::thread a([&](){
          std::this_thread::sleep_for(std::chrono::seconds(1));
          REQUIRE_NOTHROW(
                hermes::protobuf::async_send<com::Message>(
                  "127.0.0.1", "50501", message,
                  [](std::size_t bytes) {
                    std::cout << "bytes: " << bytes << std::endl;
                    REQUIRE(bytes == 30);
                  }
          ));
        });

        std::thread b([](){
          hermes::protobuf::async_receive<com::Message>(
            "50501",
            [](com::Message message){
              std::string s;
              message.SerializeToString(&s);
              std::cout << "async receive: " << s << std::endl;
            }
          );
        });

        a.join();
        b.join();
      }

  }
}
