// include tests files
#include "protobuff.cpp"
#include "messenger.cpp"

int main()
{
  //Testing Hermes protobuf operations
  std::cout << "########### TEST HERMES PROTOBUF OPERATIONS ###########" << "\n";
  test_protobuf_synchronous_operations();
  test_protobuf_asynchronous_operations();
  std::cout << "protobuf: [ok]" << "\n";

  // testing Hermes session
  std::cout << "################# TEST HERMES SESSION #################" << "\n";
  test_session();
  std::cout << "session: [ok]" << "\n";

  // testing Hermes stream
  std::cout << "################## TEST HERMES STREAM #################" << "\n";
  test_stream();
  std::cout << "stream: [ok]" << "\n";

  // testing Hermes Messenger
  std::cout << "############# TEST HERMES MESSENGER CTOR ##############" << "\n";
  test_messenger_constructor();
  std::cout << "Messenger ctor: [ok]" << "\n";
  return 0;
}
