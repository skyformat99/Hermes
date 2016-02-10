#define CATCH_CONFIG_MAIN
#include "catch.hpp"
#include "v2.hpp"

#include "Communication.pb.h"

using namespace hermes::core;

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
        ">>> should not be stuck") {
      service.run();
      REQUIRE(test);
    }

    WHEN(
        "Reset work object to run the service in our main thread."
        ">>> should not be stuck") {
      test = false;
      service.get_work().reset();
      service.get().run();
      REQUIRE(not test);
    }

    WHEN(
        "Testing post() method using a thread pool."
        ">>> no error should be thrown") {
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
        ">>> no error should be thrown") {
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
