Hermes version 1.0

# Hermes

> _Hermes is an Olympian god in Greek religion and mythology, he is described
as quick and cunning._

> _Moving freely between the worlds of the mortal and the divine, he is portrayed as emissary and the messenger of the gods._

> _Hermes suffers from no limit. He loves to outwit other gods for his own satisfaction or for the sake of humankind._


#### What is it ?

Hermes is an attempt to provide to all new developers, a user-friendly C++ support enabling communication through socket. Hermes is based on  [`Asio`](http://think-async.com/Asio) which is a cross-platform C++ library for network.
In order to meet user's wishes, Hermes is able to handle some famous serialization protocols and thus can be used in several kind of c++ program wherein network communication is needed.
As i said, Hermes has been designed to be pretty simple to use, you will need to write only a few lines to do what you want.


#### Features & Usefulness

- network:
  As Hermes is based on Asio, Hermes could not only provide a network software (client, server),
  but he is supporting TCP, UDP and could easily do asynchronous operations thanks to Asio's asynchronous model.

- serialization:
  - [`protobuf`](https://github.com/google/protobuf)
  - [`flatbuffer`](https://github.com/google/flatbuffers)

  Above, the serialization protocols handled by Hermes.

- Modularity:
  Hermes is split into modules that allows user to choose only the desired one and include it
  instead including the complete library. Furthermore, it makes easier the new feature
  implementation.

## Requirements
- c++11

The following libraries are required to use all Hermes features but they are included into Hermes repository if you do not have them.
- Asio without-boost
- Google Protocol Buffers
- Google FlatBuffers

## Compiling
Hermes is a header only library, so there is nothing to build and to link with your program.

 - Download Hermes:

```bash
  git clone --recursive https://github.com/TommyStarK/Hermes.git
```


## Documentation

You will find a full commmented documentation, including several examples, here [`Hermes Doc`](https://github.com/TommyStarK/Hermes/blob/master/DESIGN.md).


## Build status

- Travis: [![Build Status](https://travis-ci.org/TommyStarK/Hermes.svg?branch=master)](https://travis-ci.org/TommyStarK/Hermes)


## Contribution

Each Contribution is welcomed and encouraged. We do not claim to cover each use cases nor completely master the c++. If you encounter a non sense or any trouble, you can open an issue
and we will be happy to discuss about it. Do not hesitate to open an issue, if you would like to see a specific feature.


## Thanks

Thanks to my mate and friend [`Manu`](https://github.com/chambo-e) for the original idea :)


> **WARNING**: this repository is being actively developed. It is farm from being stable.
