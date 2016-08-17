#!/bin/sh

echo "build server ..."
g++ -o server server.cpp -std=c++11 -I /home/crs/sandbox/websocketpp -lboost_system -pthread -lboost_thread
if [[ $? -ne 0 ]]; then
    echo "fail to build server"
    exit 1
fi
echo "bulid client ..."
g++ -o client client.cpp -std=c++11 -I /home/crs/sandbox/websocketpp -lboost_system -pthread
if [[ $? -ne 0 ]]; then
    echo "fail to build client"
    exit 1
fi

