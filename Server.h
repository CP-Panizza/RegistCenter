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
using  namespace std;

#ifndef CPP_TCP_DEMO_SERVER_H
#define CPP_TCP_DEMO_SERVER_H
struct clinet_info{

};

class Server {
public:
    Server(int);
    ~Server();
    void Start(Handler *);
    void do_accept();
    void disconnect(int cfd);


private:
    SOCKET socket_fd;
    fd_set select_fd;
    struct sockaddr_in servaddr;
    map<SOCKET , string> clients;
};


#endif //CPP_TCP_DEMO_SERVER_H
