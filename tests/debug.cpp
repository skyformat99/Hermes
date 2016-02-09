#define CATCH_CONFIG_MAIN
#include "catch.hpp"
#include "v2.hpp"

#include "Communication.pb.h"

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
  }
}
