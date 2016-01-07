// include tests files
#include "protobuff.cpp"

int main()
{
  test_protobuf_synchronous_operations();
  test_protobuf_asynchronous_operations();
  test_netcat();
  return 0;
}
