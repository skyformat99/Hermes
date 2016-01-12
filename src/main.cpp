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
  std::cout << message << "\n\n";
}

int main()
{
  //Testing Hermes protobuf operations
  display("Serialization", "protobuf", "Testing hermes protobuf operations");
  test_protobuf_synchronous_operations();
  std::cout << "\n ***  synchronous reading/writting operations: [ok]" << "\n";
  test_protobuf_asynchronous_operations();
  std::cout << "\n *** asynchronous reading/writting operations: [ok]" << "\n";

  // testing Hermes session
  display("Messenger", "Session", "Testing session funcitonalities");
  test_session();
  std::cout << "\n *** session: [ok]" << "\n";

  // testing Hermes stream
  display("Messenger", "Stream", "Testing stream");
  test_stream();
  std::cout <<"\n *** stream: [ok]" << "\n";

  // testing Hermes Messenger
  display("Messenger", "Class", "Testing Messenger class");
  test_messenger_constructor();
  std::cout << "\n *** Messenger ctor: [ok]" << "\n";
  return 0;
}
