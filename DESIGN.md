# Hermes design

In this document, we will define each feature of Hermes. We will describe how to use it.
Let's begin with an overview of Hermes core functionalities.

Hermes has two different parts:
- Messenger:
    A global entity which allows you to create network softwares. A messenger could be a client or a server and it handles natively TCP/UDP protocol as well as asynchronous operations.

- Serialization:
    Hermes is a fast and easy way to send and receive serialized data through socket.
    Thanks to Google Protocol Buffers and FlatBuffers, Hermes could send/receive serialized
    data.
    Note: JSON is not handled by default, because all famous library has a to_string() method.


## Serialization

The purpose of Hermes serialization part is to provide a really simple use of sending/receiving serialized data. You just have to write a single line to send or receive a serialized object (only to send and receive not to create the object itself or to set its data).

#### protobuf

Google Protocol Buffers are a language-neutral, platform-neutral, mechanism
for serializing structured data.

Here, i want to point the 'structured data', that means we clearly have to use a communication protocol which allows us to be sure to get all data in the right order. Without this, it will be impossible to unserialize the message and get data back.

So for obvious reasons, Hermes network operations about protobuf will always use TCP protocol.

One more thing, as you should know, using protobuf involves having defined a .proto model and generate according classes. So let's assume that our protobuf package and message model are respectively named 'package' and 'message'...I know i'm quite imaginative :)

  If you do need help about using protobuf : [`Google Protocol Buffers doc`](https://developers.google.com/protocol-buffers/?hl=en)


##### Hermes::protobuf prototype functions

```c++
  // Synchronous operations
  template <typename T>
  std::size_t send(const std::string& host,
                   const std::string& port,
                   std::shared_ptr<T> message);

  template <typename T>
  std::shared_ptr<T> receive(const std::string& port);

  // Asynchronous operations
  typedef std::function<void(std::size_t)> callback;

  template <typename T>
  void async_send(const std::string& host,
                  const std::string& port,
                  std::shared_ptr<T> message,
                  const callback& handler = nullptr);

  typedef std::function<void(const std::string&)> rcallback;

  template <typename T>
  void async_receive(const std::string& host,
                     T * const message,
                     const rcallback& handler = nullptr);

```
##### Examples

- Example 1: Synchronous send/receive operations.

```c++
{
  #include "Hermes.hpp"

  #include <string>
  #include <memory>
  #include <iostream>

  using namespace Hermes;

  // we start by declaring our protobuf message
  auto message = std::make_shared<package::message>();

  // Once data set, we could send it.
  auto size = protobuf::send<package::message>("127.0.0.1", "8080", message);

  // 'send' function returns 0 on error and the number of bytes sent on success.
  if (not size)
    std::cerr << "Error :(" << std::endl;
  else
    std::cout << "Message sent :)" << std::endl;

  // Similarly, if you want to receive a protobuf message
  auto response = protobuf::receive<package::message>("8080");

  if (not response)
    std::cerr << "Erreur :(" << std::endl;

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

  // We start by declaring our protobuf message
  auto message = std::make_shared<package::message>();

  // Asynchronous send of a serialized version of our message without providing callback.
  protobuf::async_send("127.0.0.1", "8080", message);

  // Asynchronous send of a serialized version of our message with callback
  // given as lambda in parameters.
  protobuf::async_send("127.0.0.1", "8080", message, [](std::size_t bytes){

    std::cout << "bytes sent: " << bytes << "\n";
    // do some stuff
  });

  // Predict ending of asynchronous operations is impossible that's why 'async_send' cannot  
  // return a std::shared_ptr like its counterpart 'send'.
  // For this reason, you have to provide a raw pointer of a protobuf message using get() method
  // of std::shared_ptr to 'async_receive'.
  // There are two cases. First, you do not provide a callback, the protobuf message
  // parses the string received when the internal asynchronous handler is called by
  // the receive operation. If you provide one, you could do the parsing in your function.

  auto response = std::make_shared<package::message>();

  protobuf::async_receive("8080", response);

  if (not response)
    std::cout << "data not received yet" << "\n";


  protobuf::async_receive("8080", response [](const std::string& res){

    response->ParseFromString(res);
    // do some stuff
  });

}

```
I hope these examples will help. Take a look to the protobuf tests, it may be usefull
[`protobuf test`](https://github.com/TommyStarK/Hermes/blob/master/tests/protobuff.cpp).
