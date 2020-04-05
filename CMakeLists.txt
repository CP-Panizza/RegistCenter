cmake_minimum_required(VERSION 3.12)
project(RegistCenter)


set(CMAKE_CXX_STANDARD 11)

IF(CMAKE_SYSTEM_NAME MATCHES "Windows")
link_libraries("D:/MinGW64/x86_64-w64-mingw32/lib/libws2_32.a")
ENDIF()

add_executable(RegistCenter main.cpp MyHandle.cpp MyHandle.h Server.cpp Server.h Handler.h RWLock.hpp
        libs/http/HttpServer.cpp
        libs/http/HttpServer.h
        libs/http/Request.cpp
        libs/http/Request.h
        libs/http/Response.h
        libs/http/Response.cpp
        libs/http/util.cpp
        libs/http/util.h
        libs/my_socket.h
        libs/my_socket.h)
