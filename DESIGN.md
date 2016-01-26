# Hermes design

In this document, we will define each feature of Hermes.
We will describe how to use it. Let's begin with an overview of Hermes core functionalities.

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

  As i said earlier, Messenger allows you to create network softwares. You have
  to define what kind of software you want. Client and server, both alike, could send
  and receive data following TCP or UDP protocol (this also you will have to specify).
  Messenger has been designed to be pretty simple to use, and you will have only a few lines
  to write to perform operations.
  Thanks to a boolean, you can specify to your messenger if it has to handle asynchronous
  operations.
  Below, you will find how to declare a Messenger, available methods that you could use to
  manipulate your software and many examples.


#### Constructor

```c++
  #include "Hermes.hpp"

  using namespace Hermes;

  Messenger(const std::string& software,
            const std::string& protocol,
            bool async,
            const std::string& port,
            const std::string& host = "127.0.0.1");
```

#### Software Methods

```c++
    #include "Hermes.hpp"

    using namespace Hermes;

    // I will show you how to use these methods appropriately
    // according the different cases.
    void run();
    void disconnect();

    // Synchronous send/receive operation.
    std::size_t send(const std::string& message);
    std::string receive();

    // asynchronous send/receive operation
    void async_send(const std::string& message,
                    const std::function<void(std::size_t)>& callback = nullptr);

    void async_receive(const std::function<void(std::string)>& callback);

    // Same as run/disconnect i would come back on this later
    void set_connection_handler(const std::function<void()>& handler);
    void set_disconnection_handler(const std::function<void()>& handler);


    // NOTE: to perform asynchronous operations you have to specify it to your messenger.
```

#### Synchronous softwares.

- Exemple 1: TCP Client

```c++
  #include "Hermes.hpp"

  #include <iostream>

  using namespace Hermes;

  // a Synchronous TCP client
  // no host provided, so by default it's localhost
  auto tcp_client = new Messenger("client", "tcp", false, "5050");


  auto co = [](){
    std::cout << "connected :)" << std::endl;
  };


  // Connect client
  tcp_client->run();

  // Execute a handler at the connection of the client
  tcp_client->set_connection_handler(co); // you have to set the handler before call run.
  tcp_client->run();


  // If you want your client to perform an operation at the connection
  // you could do something like this
  tcp_client->set_connection_handler([&](){
      tcp_client->send("message");
      auto message = tcp_client->receive();
  });

  tcp_client->run();

  // Following the same idea you have the possibility to set a disconnection handler.
  // It works like the connection handler and you also have to set it before call
  // the disconnection of your client.
  // NOTE: In case of error, the disconnect method is called before throwing the error
  //       so the handler too if you had set one.

  // send/receive operations
  auto bytes = tcp_client->send("message");
  auto response = tcp_client->receive();

  std::cout << "number of bytes sent by tcp: " << bytes << std::endl;
  std::cout << "message received by tcp: " << response << std::endl;
```

- Exemple 2: TCP Server

```c++
  #include "Hermes.hpp"

  #include <iostream>

  using namespace Hermes;

  // a Synchronous TCP server
  auto tcp_server = new Messenger("server", "tcp", false, "5050");


  auto co = [](){
    std::cout << "got connection :)" << std::endl;
  };


  // Start server
  tcp_server->run();

  // Execute a handler when you have a connection
  tcp_server->set_connection_handler(co); // you have to set the handler before call run.
  tcp_server->run();


  // If you want your server to perform an operation when he has a connection
  // you could do something like this
  tcp_server->set_connection_handler([&](){
      tcp_server->send("message");
      auto message = tcp_server->receive();
    });

  tcp_server->run();

  // Following the same idea you have the possibility to set a disconnection handler.
  // It works like the connection handler and you also have to set it before call
  // the disconnection of your client.
  // NOTE: In case of error, the disconnect method is called before throwing the error
  //       so the handler too if you had set one.

  // send/receive operations
  auto bytes = tcp_server->send("message");
  auto response = tcp_server->receive();

  std::cout << "number of bytes sent by tcp: " << bytes << std::endl;
  std::cout << "message received by tcp: " << response << std::endl;

```




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

  using namespace Hermes::protobuf;

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
```
- Example 2: Asynchronous send/receive operations.

```c++
  #include "Hermes.hpp"

  #include <string>
  #include <iostream>

  using namespace Hermes;

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



  protobuf::async_receive<package::message>("8080", [](com::Message response) {

      // do some stuff
      auto descriptor = response.GetDescriptor();
  });

```

Note: The socket is shutdown and close after any Hermes protobuf operations.

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


Note: All modules are in the namespace 'Hermes'.
