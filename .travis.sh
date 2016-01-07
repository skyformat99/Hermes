#! /bin/bash
mkdir build
cd build
cmake ../
make
./hermes_tests
