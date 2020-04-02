cmake_minimum_required(VERSION 3.12)
project(RegistCenter)

set(CMAKE_CXX_STANDARD 11)
link_libraries("D:/MinGW64/x86_64-w64-mingw32/lib/libws2_32.a")

add_executable(RegistCenter main.cpp MyHandle.cpp MyHandle.h Server.cpp Server.h Handler.h)
