#!/bin/sh

THRIFT_CODE='gen-cpp/HelloSevice.cpp gen-cpp/hello_constants.cpp gen-cpp/hello_types.cpp'
LIBS='-lthrift -lpthread -lglog -lboost_system'
g++ -o client client.cpp $THRIFT_CODE -std=c++11 $LIBS
g++ -o server server.cpp $THRIFT_CODE -std=c++11 $LIBS

