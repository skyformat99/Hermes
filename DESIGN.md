# Hermes design

In this document, we will define each feature of Hermes. We will describe how to use it. First of all, we will talk about how to use google protocol buffer with Hermes. Then, we will do an overview of messengers, how to create them, and how to use them.

## Hermes and protobuf

So, let's begin the first part of this documentation. How to send a google protocol buffer with Hermes ? As i said, Hermes has been designed to be pretty simple to use and could handle nativly some of the most famous serialization protocol.

The only task that the user has to do, it's to define his own model of google protocol buffer.

We will assume that we have define a protobuf package named 'pkg' wich contains a model message named 'msg'. These two names are here to make the following example more understandable.


```c++
{
  // Hermes is a header-only library.
  #include "Hermes.hpp"

  using namespace Hermes;


  // here i declare a new protobuf object
  auto message = new pkg::msg;



  // First we want to send our object 'message'.
  // The 'send' function in namespace 'protobuf' takes 3 parameters: host, port and a pointer on
  // message.
  // The 'send' function serializes the object to a string and send it to the given host through
  // a tcp socket.
  // As you can see, this function is templated so you can send any kind of protobuf.


  protobuf::send<pkg::msg>("127.0.0.1", "1234", message);


  // Respectively, the 'receive' function allows you to receive a string wich represents
  // a serialized protobuf and return the object created from the parsing of this string.

  auto response = protobuf::receive<pkg::msg>("1234");

}

```
