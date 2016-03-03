# Hermes design

In this document, we will define each feature of Hermes. We will describe how to
use it. Let's begin with an overview of Hermes core functionalities.

Hermes has three parts:

- Network softwares:

    The purpose of Hermes is to provide to his user a simple c++ library supporting
    TCP/UDP protocol, and helping the user to create and use those network softwares.
    Hermes owns two dedicated namespaces to permit his user to construct client
    and server following TCP or UDP protocol.

- Serialization:

    Hermes is a, fast and easy, way to send and receive serialized data through
    a socket. Thanks to Google Protocol Buffers and FlatBuffers, Hermes could
    send/receive serialized data.

Note: JSON is not handled by default because all famous library
      has a to_string() method.


- Modules:

    Hermes is split into modules, you do not have to use all features if you do not want to.
    That's why, you will find them into the repository include/modules.




## Network softwares

Below, you will find usages examples to show you how to create and use Hermes
network softwares.


#### TCP


- Client


```c++

  #include "Hermes.hpp"

  //
  // First let's see the synchronous way
  //

  // TCP Client
  hermes::tcp::Client client("127.0.0.1", "50501");


  // connecting the client to the given remote (host:port)
  client.connect();

  // send data to the server
  std::size_t bytes = client.send("here a message from my client :)");


  // waiting for data
  // this is a blocking function. It means that your program
  // will be stuck until there is at least one byte received.
  std::string received = client.receive();


  // disconnecting your client
  client.disconnect();

  // NOTE: disconnect method is automatically called in the client destructor
  //       if this one is still connected when the object is about to be destroyed.



  //
  // Now, let's have a look to the asynchronous side.
  //


  // asynchronous connection
  //
  //  @param:
  //    - callback
  //
  //  A callback could be provided and it will be invoked when the asynchronous
  //  connection will be completed.

  client.async_connect(); // no callback provided.

  // here we pass a lambda as callback to the async_connect method.
  // it will be invoked and executed once the asynchronous connection is performed.
  client.async_connect([](hermes::network::Stream& session){

    // do some stuff at the connection.
    // you could decide to do operations at the connection.
    // To do that use the session (param) as you used your client.

    // e.g: i want to send and receive data once my client is connected:
    session.send("data");
    std::cout << session.receive() << std::endl;
  });


  // For the following functions, you will need to set handlers before using them.
  // I explain myself, as you should know asynchronous operation means that the operation
  // wont be performed when you call it. So we generally use a handler that we pass to our
  // asynchronous operation and it is invoked when the operation is performed.


  // asynchronous send

  auto send_handler = [](std::size_t bytes, /*number of bytes send*/
                         hermes::network::Stream& session/*the current connection*/){
     // do some stuff.
  };


  client.set_send_handler(send_handler);
  client.async_send("here a message from my client :).");

  // same for the async receive function

  auto receive_handler = [](std::string received, /*the data received*/
                         hermes::network::Stream& session/*the current connection*/){
     // do some stuff.
  };


  client.set_receive_handler(receive_handler);
  client.async_receive();


  // disconnection
  client.disconnect();
```


- Server

  comming soon.


```c++

    #include "Hermes.hpp"

```

#### UDP



## Serialization

The purpose of Hermes serialization part is to provide a really simple use of sending/receiving serialized data. You just have to write a single line to send or receive a serialized object (only to send and receive not to create the object itself or to set its data).


#### protobuf


Google Protocol Buffers are a language-neutral, platform-neutral, mechanism
for serializing structured data.

Here, i want to point the 'structured data', that means we clearly have to use a communication protocol which allows us to be sure to get all data in the right order. Without this, it will be impossible to unserialize the message and get data back.

So for obvious reasons, Hermes network operations about protobuf will always use TCP protocol.

One more thing, as you should know, using protobuf involves to having defined a .proto model and generated according classes. So let's assume that our protobuf package and message model are respectively named 'package' and 'message'...I know i'm quite imaginative :)

  If you do need help about using protobuf : [`Google Protocol Buffers doc`](https://developers.google.com/protocol-buffers/?hl=en)


##### Prototype functions

```c++
  #include "Hermes.hpp"

  using namespace hermes::protobuf;

  // Synchronous operations
  template <typename T>
  std::size_t send(const std::string& host,
                   const std::string& port,
                   const T& message);

  template <typename T>
  T receive(const std::string& port);

  // Asynchronous operations
  template <typename T>
  void async_send(const std::string& host,
                  const std::string& port,
                  const T& message,
                  const std::function<void(std::size_t)>& callback = nullptr);

  template <typename T>
  void async_receive(const std::string& port,
                     const std::function<void(T)>& callback);

```

##### Design - Examples

- Example 1: Synchronous send/receive operations.

```c++
  #include "Hermes.hpp"
  // you have to include the generated header of the protobuf class

  using namespace hermes;

  package::message message;

  // set data to message

  auto size = protobuf::send<package::message>("127.0.0.1", "8080", message);

  // 'send' function returns 0 on error, and the number of bytes sent on success.
  if (not size)
    std::cerr << "Error :(" << std::endl;
  else
    std::cout << "Message sent :)" << std::endl;


  // 'receive' functions returns a T object
  // with T the type of your protobuf message
  auto response = protobuf::receive<package::message>("8080");

  auto descriptor = response.GetDescriptor();
```


- Example 2: Asynchronous send/receive operations.



```c++
  #include "Hermes.hpp"
  // you have to include the generated header of the protobuf class

  using namespace hermes;

  package::message message;

  // set data to message.

  // Asynchronous send without providing callback.
  protobuf::async_send<package::message>("127.0.0.1", "8080", message);

  // With callback given as lambda.
  protobuf::async_send<package::message>("127.0.0.1", "8080", message,
                                          [](std::size_t bytes) {

    std::cout << "bytes sent: " << bytes << "\n";
    // do some stuff
  });


  // Asynchronous receive
  protobuf::async_receive<package::message>("8080", [](package::message response) {

      // do some stuff
      auto descriptor = response.GetDescriptor();
  });

```

Note: The socket is shutdown and close after any Hermes protobuf operations.

Take a look to the protobuf tests, it may be usefull:
[`protobuf test`](https://github.com/TommyStarK/Hermes/blob/master/tests/protobuff.cpp).





#### flatbuffers


coming soon.







## Modules

  In the repository include/modules, you will find the following modules:

  - Hermes_protobuf.hpp
  - Hermes_tcp_client.hpp
  - more soon.



 A module works as Hermes. The modules are headers only so, you just have to
  include the desired one to use it.



  ```c++
    // For example, i just want to use the Hermes protobuf operations.
    #include "Hermes_protobuf.hpp"
  ```


Note: All modules are in the namespace 'hermes'.
