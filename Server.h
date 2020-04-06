//
// Created by Administrator on 2020/3/6.
//
#include<cstdio>
#include<cstdlib>
#include<cstring>
#include<cerrno>
#include <map>
#include<sys/types.h>
#include "libs/my_socket.h"
#include<unistd.h>
#include "Handler.h"
#include <thread>

#ifdef _WIN64

#else

#include <sys/epoll.h>

#endif

#ifndef _MAX_COUNT_
#define MAX_COUNT 1024
#endif

#ifndef CPP_TCP_DEMO_SERVER_H
#define CPP_TCP_DEMO_SERVER_H
using namespace std;

class Server {
public:
    Server(int);

    ~Server();

    void Start(Handler *);

    void do_accept();

    void disconnect(int cfd);

private:
#ifdef _WIN64
    SOCKET socket_fd;
    fd_set select_fd;
    map<SOCKET , string> clients;
#else
    int socket_fd;
    struct epoll_event *epoll_events;
    int epoll_fd;
    map<int, string> clients;
#endif
};


#endif //CPP_TCP_DEMO_SERVER_H
