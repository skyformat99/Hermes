#define CATCH_CONFIG_MAIN
#include "catch.hpp"
#include "v2.hpp"

#include "Communication.pb.h"

// std::mutex mutex;

using namespace hermes;

SCENARIO("Service", "[core]") {
  GIVEN("Service object") {
    bool test = true;
    core::Service service;

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
      auto f = [&](size_t n) {
        {
          std::mutex mutex;
          std::lock_guard<std::mutex> lock(mutex);
          std::cout << "[" << std::this_thread::get_id() << "] " << n
                    << std::endl;
        }

        std::this_thread::sleep_for(std::chrono::milliseconds(1000));
        for (int i = 0; i < 100; ++i) n += i;

        {
          std::mutex mutex;
          std::lock_guard<std::mutex> lock(mutex);
          std::cout << "[" << std::this_thread::get_id() << "] " << n
                    << std::endl;
        }
      };

      auto routine = [](asio::io_context& service) { service.run(); };

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
  }
}
