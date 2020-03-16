cmake_minimum_required(VERSION 3.12)
project(RegistCenter)

set(CMAKE_CXX_STANDARD 14)


add_executable(RegistCenter main.cpp MyHandle.cpp MyHandle.h Server.cpp Server.h interface/Handler.h)
