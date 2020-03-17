//
// Created by Administrator on 2020/3/6.
//

#ifndef CPP_TCP_DEMO_MYHANDLE_H
#define CPP_TCP_DEMO_MYHANDLE_H

#include <list>
#include <mutex>
#include <map>
#include "interface/Handler.h"
#include <thread>
#include <sys/socket.h>
#include <cstdio>
#include <unistd.h>
#include <cstring>
#include <iostream>
using namespace std;
#define MAXLINE 4096
#define CLIENT_PORT 8528

class MyHandle : public Handler {
private:
    mutex my_mutex;
    map<string, list<string> *> server_list_map;
public:
    MyHandle();
    ~MyHandle();
    void Server(int connfd, string remoteIp, int port);
    void HeartCheck();
    void HeartCheckEntry();
    bool DoCheck(const string &ip);
    void DeleteAddr(string ip);
};

bool count(const list<string> &l, string target);

#endif //CPP_TCP_DEMO_MYHANDLE_H
