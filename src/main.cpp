// include tests files
#include "protobuff.cpp"
#include "messenger.cpp"

int main()
{
  // Module - Messenger
  // testing all parts
  test_session();
  test_stream();
  test_tcp_protocol();
  test_udp_protocol();


  // Module - Serialization
  //Testing Hermes protobuf operations
  test_protobuf_synchronous_operations();
  test_protobuf_asynchronous_operations();
  return 0;
}
