# Hermes design

In this document, we will define each feature of Hermes. We will describe how to use it.
Let's begin with an overview of Hermes design.

Hermes has two different parts:
- Messenger:
    A global entity which allows you to create various network softwares. Each messenger has his own methods to do the job. Messenger could do asynchronous operations. By operations i mean, connect or accept a socket, send and receive data, execute callbacks, handle disconnect etc.

- Serialization:
    Hermes is a fast and easy way to send/receive serialized data through socket.
    You will find various namespaces into Hermes, they are here to separate the differents serialization protocols, respectively 'protobuf', 'flatbuffer' and 'binary'.


## Serialization

The purpose of Hermes serialization part is to provide a really simple use of sending/receiving serialized data. You just have to write a single line to send/receive a serialized object (only to send and receive not to create the object itself or to set its data).

#### protobuf

Google Protocol Buffers are a language-neutral, platform-neutral, mechanism for serializing structured data.
Here, i want to point the 'structured data', that means we clearly have to use a communication protocol which allows us to be sure to get all data and in the right order. Without this, it will be impossible to unserialize object and get data back.
So for obvious reasons, Hermes network operations about protobuf will always use TCP protocol.

One more thing, as you should know, using protobuf needs to have defined a .proto model and generate according classes. So let's assume that our protobuf package is name 'package' and our message model named 'msg'.I know i'm quite imaginative :)

  if you need help about using protobuf : [![Google Protocol Buffers doc](https://developers.google.com/protocol-buffers/?hl=en)]

- Example: Synchronous sending/receiving protobuf object.

```c++
{
  #include "Hermes.hpp"

  #include <iostream>

  using namespace Hermes;

  // Here the following prototypes in Hermes::protobuf, the template T represents your protobuf
  // message model, in our example package::message. Function send returns number of bytes sent.
  template<typename T>
  std::size_t send(const std::string& host, const std::string& port, T* message);

  // receive returns a pointer to an object of type T created by parsing the string received
  // through TCP socket. This string is a protobuf object serialized.
  template<typename T>
  T* receive(const std::string& port);


  // we start by declaring our protobuf object
  auto protobuf_object = new package::msg;

  // Once we have set the according data which want to be sent. we call the send function.
  // In case of error, 0 is returned.
  auto error = protobuf::send<package::message>("127.0.0.1", "8080", protobuf_object);


  if (not error)
    std::cerr << "Error :(" << std::endl;
  else
    std::cout << "Message sent :)" << std::endl;


  // ok, now we want to receive a protobuf serialized object.
  auto receive_protobuf_object = protobuf::receive<package::message>("8080");

  if (receive_protobuf_object == nullptr)
    std::cerr << "Erreur :(" << std::endl;

}

```
