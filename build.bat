@echo off

mkdir .\build\release
cd build\release

clang++ -std=c++20 -O3 -DNDEBUG -Wall -Wextra .\..\..\main.cpp -o main
