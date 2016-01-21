#define CATCH_CONFIG_MAIN
#include "catch.hpp"
#include "Hermes.hpp"


SCENARIO("Hermes Messenger", "[TCP connection]") {
    GIVEN("Synchronous client connection to a server which is not listenning") {
        auto client = new Hermes::Messenger("client", "tcp", false, "2222");

        WHEN("you try to connect") {
            REQUIRE_THROWS(client->run());
        }
    }


    GIVEN("Synchronous client connection to available server") {
        auto client = new Hermes::Messenger("client", "tcp", false, "2222");
        auto server = new Hermes::Messenger("server", "tcp", true, "2222");

        server->set_connection_handler([&](){
          server->disconnect();
        });
        WHEN("you try to connect") {
            REQUIRE_NOTHROW(client);
            REQUIRE_NOTHROW(server);
            REQUIRE_NOTHROW(client->run());
            REQUIRE_NOTHROW(client->disconnect());
            REQUIRE_NOTHROW(server->run());
        }
    }
  }
