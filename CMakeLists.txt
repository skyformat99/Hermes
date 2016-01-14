# Minimum cmake version required
cmake_minimum_required(VERSION 3.2.0 FATAL_ERROR)

# Project name
set(PROJECT Hermes)

# Version
set(HERMES_VERSION_MAJOR 0)
set(HERMES_VERSION_MINOR 1)
set(HERMES_VERSION_PATCH 0)
set(HERMES_VERSION_STRING ${HERMES_VERSION_MAJOR}.${HERMES_VERSION_MINOR}.${HERMES_VERSION_PATCH})

# protobuf
find_package(Protobuf REQUIRED)
include_directories(${PROTOBUF_INCLUDE_DIRS})

# Build
if(NOT CMAKE_CONFIGURATION_TYPES AND NOT CMAKE_BUILD_TYPE)
   set(CMAKE_BUILD_TYPE Release)
endif(NOT CMAKE_CONFIGURATION_TYPES AND NOT CMAKE_BUILD_TYPE)

set(HERMES_LIBRARIES hermes CACHE INTERNAL "")
set(HERMES_INCLUDE_DIRS ${CMAKE_CURRENT_SOURCE_DIR}/include CACHE INTERNAL "")

# Compilation
if ("${CMAKE_CXX_COMPILER_ID}" STREQUAL "GNU" OR
    "${CMAKE_CXX_COMPILER_ID}" STREQUAL "Clang")
  set(warnings "-Wall -Wextra")
elseif ("${CMAKE_CXX_COMPILER_ID}" STREQUAL "MSVC")
  set(warnings "/W4 /WX /EHsc")
endif()

include(CheckCXXCompilerFlag)
CHECK_CXX_COMPILER_FLAG("-std=c++14" COMPILER_SUPPORTS_CXX14)
CHECK_CXX_COMPILER_FLAG("-std=c++11" COMPILER_SUPPORTS_CXX11)
if(COMPILER_SUPPORTS_CXX14)
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++14")
elseif(COMPILER_SUPPORTS_CXX11)
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11")
else()
  message(STATUS "The compiler ${CMAKE_CXX_COMPILER} has no C++11/14 support. Please use a different C++ compiler.")
endif()

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wno-ignored-qualifiers -Wno-unused-parameter")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wno-unused-variable -lpthread -g")

include_directories(pb)
include_directories(tests)
include_directories(include)
include_directories(libs/asio/asio)
include_directories(libs/asio/asio/include)


add_executable(test_hermes pb/Communication.pb.cc
                           src/main.cpp)

target_link_libraries(test_hermes ${Boost_FILESYSTEM_LIBRARY}
                                   ${Boost_SYSTEM_LIBRARY}
                                   ${PROTOBUF_LIBRARY})
