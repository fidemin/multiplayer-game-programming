#!/usr/bin/env bash

playerName="${1:-Player}"
echo "Starting client with player name: ${playerName}"

g++ -std=c++17 -o client.o examples/client.cpp
./client.o ${playerName} 