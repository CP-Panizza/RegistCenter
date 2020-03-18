//
// Created by Administrator on 2020/3/6.
//
#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include<errno.h>
#include<sys/types.h>
#include<sys/socket.h>
#include<netinet/in.h>
#include<unistd.h>
#include "interface/Handler.h"
#include <thread>
using  namespace std;

#ifndef CPP_TCP_DEMO_SERVER_H
#define CPP_TCP_DEMO_SERVER_H


class Server {
public:
    Server(int);
    ~Server();
    void Init();
    void Start(Handler *);
    void Accept(Handler *);
    void Close();


private:
    int listenfd;
    struct sockaddr_in servaddr;
};


#endif //CPP_TCP_DEMO_SERVER_H
