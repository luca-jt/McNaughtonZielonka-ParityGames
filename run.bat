@echo off

mkdir .\build
cd build

clang++ -std=c++20 -O3 -DNDEBUG -Wall -Wextra .\..\main.cpp -o main

.\main.exe
