// include tests files
#include "protobuff.cpp"
#include "messenger.cpp"

int main()
{
  // Module - Serialization
  //Testing Hermes protobuf operations
  test_protobuf_synchronous_operations();
  test_protobuf_asynchronous_operations();



  // Module - Messenger
  test_session();
  test_stream();

  // testing Hermes Messenger
  // test_messenger_client();
  std::cout << "*** Messenger client: [ok]" << "\n";
  return 0;
}
