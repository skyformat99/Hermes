# Hermes design

In this document, we will define each feature of Hermes. We will describe how to use it.
Let's begin with an overview of Hermes core functionalities.

Hermes has three different parts:
- Messenger:
    A global entity which allows you to create a network software. A messenger could be a client or a server and it handles natively TCP/UDP protocol as well as asynchronous operations.

- Serialization:
    Hermes is a, fast and easy, way to send and receive serialized data through a socket.
    Thanks to Google Protocol Buffers and FlatBuffers, Hermes could send/receive serialized
    data.

Note: JSON is not handled by default, because all famous library has a to_string() method.

- Modules:
    Hermes is split into modules, you do not have to use all features if you do not want to.
    That's why, you will find them into the repository include/modules.


## Messenger
  Incoming.



## Serialization

The purpose of Hermes serialization part is to provide a really simple use of sending/receiving serialized data. You just have to write a single line to send or receive a serialized object (only to send and receive not to create the object itself or to set its data).

#### protobuf

Google Protocol Buffers are a language-neutral, platform-neutral, mechanism
for serializing structured data.

Here, i want to point the 'structured data', that means we clearly have to use a communication protocol which allows us to be sure to get all data in the right order. Without this, it will be impossible to unserialize the message and get data back.

So for obvious reasons, Hermes network operations about protobuf will always use TCP protocol.

One more thing, as you should know, using protobuf involves to having defined a .proto model and generated according classes. So let's assume that our protobuf package and message model are respectively named 'package' and 'message'...I know i'm quite imaginative :)

  If you do need help about using protobuf : [`Google Protocol Buffers doc`](https://developers.google.com/protocol-buffers/?hl=en)


##### Hermes::protobuf prototype functions

```c++
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
##### Desgign - Examples

- Example 1: Synchronous send/receive operations.

```c++
{
  #include "Hermes.hpp"

  #include <string
  #include <iostream>

  using namespace Hermes;

  package::message message;

  // set data to message

  auto size = protobuf::send<package::message>("127.0.0.1", "8080", message);

  // 'send' function returns 0 on error, and the number of bytes sent on success.
  if (not size)
    std::cerr << "Error :(" << std::endl;
  else
    std::cout << "Message sent :)" << std::endl;


  auto response = protobuf::receive<package::message>("8080");

  // Do stuff with response
  auto descriptor = response.GetDescriptor();
}

```
- Example 2: Asynchronous send/receive operations.

```c++
{
  #include "Hermes.hpp"

  #include <string>
  #include <memory>
  #include <iostream>

  using namespace Hermes;

  package::message message;

  // set data to message.

  // Asynchronous sending without providing callback.
  protobuf::async_send<package::message>("127.0.0.1", "8080", message);

  // With callback given as lambda.
  protobuf::async_send<package::message>("127.0.0.1", "8080", message,
                                          [](std::size_t bytes) {

    std::cout << "bytes sent: " << bytes << "\n";
    // do some stuff
  });



  protobuf::async_receive<package::message>("8080", [](com::Message response) {

      // do some stuff
      auto descriptor = response.GetDescriptor();
  });

}

```
Take a look to the protobuf tests, it may be usefull:
[`protobuf test`](https://github.com/TommyStarK/Hermes/blob/master/tests/protobuff.cpp).


#### flatbuffers
Inconming.



## Modules

  In the repository include/modules, you will find the following modules:
  - Hermes_protobuf.hpp
  - Hermes_messenger.hpp
  - Hermes_flatbuffers.hpp

  A module works as Hermes. The modules are headers only so, you just have to
  include the desired one to use it.

  ```c++
    // For example, i just want to use the Hermes protobuf operations.
    #include "Hermes_protobuf.hpp"
  ```

  Note: All modules are in namespace 'Hermes'.
