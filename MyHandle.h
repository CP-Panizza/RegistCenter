//
// Created by Administrator on 2020/3/6.
//

#ifndef CPP_TCP_DEMO_MYHANDLE_H
#define CPP_TCP_DEMO_MYHANDLE_H

#include <list>
#include <mutex>
#include <map>
#include "Handler.h"
#include <thread>
#include <winsock2.h>
#include <cstdio>
#include <unistd.h>
#include <cstring>
#include <iostream>
using namespace std;
#define MAXLINE 4096
#define CLIENT_PORT 8528

struct ServerInfo{
    string ip;
    int proportion;
};


class MyHandle : public Handler {
private:
    mutex my_mutex;
    map<string, list<ServerInfo> *> server_list_map;
public:
    MyHandle();
    ~MyHandle();
    void Server(int connfd, string remoteIp);
    void HeartCheck();
    void HeartCheckEntry();
    void PreCheck();
    bool DoCheck(const string &ip);
    void DeleteAddr(string ip);
};

bool count(const list<ServerInfo> &l, string target);

#endif //CPP_TCP_DEMO_MYHANDLE_H
