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

One more thing, as you should know, using protobuf needs to have defined a .proto model and generate according classes. So let's assume that our protobuf package is name 'package' and our message model named 'message'.I know i'm quite imaginative :)

  if you need help about using protobuf : [`Google Protocol Buffers doc`](https://developers.google.com/protocol-buffers/?hl=en)

- Example: Synchronous sending/receiving protobuf object.

```c++
{
  #include "Hermes.hpp"

  #include <string>
  #include <memory>
  #include <iostream>

  using namespace Hermes;

  // Here the following prototypes in Hermes::protobuf, the template T represents your protobuf
  // message model, in our example package::message. Function send returns number of bytes sent.
  template<typename T>
  std::size_t send(const std::string& host, const std::string& port, std::shared_ptr<T> message);

  // receive returns a smart_pointer to an object of type T created by parsing the string received
  // through TCP socket. This string is a protobuf object serialized.
  template<typename T>
  std::shared_ptr<T> receive(const std::string& port);


  // we start by declaring our protobuf object
  auto protobuf_object = new package::message;

  // Once we have set the according data which want to be sent. we call the send function.
  // In case of error, 0 is returned.
  auto size = protobuf::send<package::message>("127.0.0.1", "8080", protobuf_object);


  if (not size)
    std::cerr << "Error :(" << std::endl;
  else
    std::cout << "Message sent :)" << std::endl;


  // ok, now we want to receive a protobuf serialized object.
  auto receive_protobuf_object = protobuf::receive<package::message>("8080");

  if (receive_protobuf_object.get() == nullptr)
    std::cerr << "Erreur :(" << std::endl;

}

```
- Asynchronous send/receive of a protobuf object.

```c++
{
  #include "Hermes.hpp"

  #include <string>
  #include <memory>
  #include <iostream>

  using namespace Hermes;

  // Asynchronous send
  //
  // The followings prototypes will be usefull.

  typedef std::function<void()> callback;
  template <typename T>
  void aync_send(const std::string& host, const std::string& port,
                 std::shared_ptr<T> message, const callback& handler = nullptr);


   // As you can see, callback is a typedef representing a function object;
   // By default callback equals to nullptr, to allow user to provide it or not.


   auto protobuf = new package::message;

   protobuf::async_send("127.0.0.1", "8080", protobuf);


   protobuf::async_send("127.0.0.1", "8080", protobuf, [](){
      // do_some_stuff
   });

  // Asynchronously receive
  //
  typedef std::function<void(const std::string&)> rcallback;
  template <typename T>
  void async_receive(const std::string& port, std::shared_ptr<T> message,
                     const rcallback& handler = nullptr);

   auto protobuf_to_receive = new package::message;
   protobuf_to_receive = nullptr;

   // async_receive is a bit more complex, as it is impossible to predicate
   // when the asynchronous operation will end, you have to pass a shared_ptr as parameter
   // to fill it with the data.
   // This function cannot simply return the T object because of asynchronous.
   // So the default behaviour has been thought to fill the T object when the handler of the
   // asynchronous receive is called, at this moment we are sure that the receive has been
   // performed.
   // An other way is to pass a rcallback to async_receive.


   protobuf::async_receive("8080", protobuf_to_receive);

   if (not protobuf_to_receive)
      std::cout << "data not received yet" << "\n";


    protobuf::async_receive("8080", protobuf_to_receive, [](const std::string& res){

        protobuf_to_receive->ParseFromString(res);
        // do some stuff
    });

}

```
I hope this examples will help.
[`protobuf test`](https://github.com/TommyStarK/Hermes/blob/master/tests/protobuff.cpp) you could take a look to the protobuf tests, it may help.
