#define CATCH_CONFIG_MAIN
#include "catch.hpp"
#include "v2.hpp"

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
    }

    WHEN(
        "Reset work object to run the service in our main thread."
        "\n>>> should not be stuck") {
      test = false;
      service.get_work().reset();
      service.get().run();
      REQUIRE(not test);
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
  GIVEN("Error class, 3 object functions to test the different error type") {
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

SCENARIO("Stream session", "[network]") {
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
      session->service().get_work().reset();
      if (session->service().get_thread().joinable())
        session->service().get_thread().join();
    }

    WHEN(
        "connecting tcp socket on local to throw an error "
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
      session->service().get_work().reset();
      if (session->service().get_thread().joinable())
        session->service().get_thread().join();
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
      REQUIRE_NOTHROW(session->connect(endpoint));

      session->service().run();

      std::vector<std::thread> threads;
      for (int i = 0; i < 100; i++)
        threads.push_back(std::thread(std::bind(fct, session)));

      for (int i = 0; i < 100; i++) threads[i].join();

      session->service().get_work().reset();
      if (session->service().get_thread().joinable())
        session->service().get_thread().join();
      REQUIRE(not session->is_connected());
    }

    // testing with netcat
    // the two followings functions are commented to avoid the failure
    // of running tests on travis as there is no server listenning on port 9999

    // WHEN("sending tcp message to netcat on port 9999.") {
    //   auto session = Stream::new_session(service);
    //
    //   auto run_session = [&](){
    //     session->service().run();
    //   };
    //
    //   auto stop_session = [&]() {
    //     session->disconnect();
    //     session->service().get_work().reset();
    //     if (session->service().get_thread().joinable())
    //       session->service().get_thread().join();
    //   };
    //
    //   run_session();
    //   asio::ip::tcp::resolver resolver(service.get());
    //   asio::ip::tcp::resolver::query query("127.0.0.1", "9999");
    //   asio::ip::tcp::endpoint endpoint = *resolver.resolve(query);
    //   REQUIRE_NOTHROW(session->connect(endpoint));
    //   session->send("test :)");
    //   auto response = session->receive();
    //   std::cout << response << std::endl;
    //   stop_session();
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
    //   for (int i = 0; i < 100; i++)
    //     threads.push_back(std::thread(std::bind(fct, session)));
    //
    //   for (int i = 0; i < 100; i++) threads[i].join();
    //
    //   session->disconnect();
    //   session->service().get_work().reset();
    //   if (session->service().get_thread().joinable())
    //     session->service().get_thread().join();
    //   REQUIRE(not session->is_connected());
    // }
  }
}
