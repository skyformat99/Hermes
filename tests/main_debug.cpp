#include "v2.hpp"

#include "Communication.pb.h"

using namespace hermes;

void test(asio::io_context::strand& strand) { (void)strand; std::cout << "debug\n"; }

int main() {

  core::Service service;

  test(service.get_strand());

  std::unique_ptr<int> toto;

  toto = std::unique_ptr<int>(new int(4));

  std::cout << *toto  << std::endl;
  return 0;
}
