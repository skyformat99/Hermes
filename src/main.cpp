// include tests files
#include "protobuff.cpp"
#include "messenger.cpp"

void display(const std::string& module,
            const std::string& description,
            const std::string& message) {

  std::cout << "\n";
  std::cout << "##################################################" << "\n";
  std::cout << "#               !! TEST !!                       #" << "\n";
  std::cout << "##################################################" << "\n";
  std::cout << "Module: " << module << " => " << description << "\n";
  std::cout << message << "\n";
}

int main()
{
  //Testing Hermes protobuf operations
  display("Serialization", "protobuf", "Testing hermes protobuf operations");
  test_protobuf_synchronous_operations();
  std::cout << "***  synchronous reading/writting operations: [ok]" << "\n";
  test_protobuf_asynchronous_operations();
  std::cout << "*** asynchronous reading/writting operations: [ok]" << "\n";

  // testing Hermes session
  display("Messenger", "Session", "Testing session funcitonalities");
  test_session();
  std::cout << "*** session: [ok]" << "\n";

  // testing Hermes stream
  display("Messenger", "Stream", "Testing stream");
  test_stream();
  std::cout <<"*** stream: [ok]" << "\n";

  // testing Hermes Messenger
  display("Messenger", "Class", "Testing Messenger class");
  // test_messenger_client();
  std::cout << "*** Messenger client: [ok]" << "\n";
  return 0;
}
