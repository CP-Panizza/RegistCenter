//
// Created by Administrator on 2020/3/6.
//

#ifndef CPP_TCP_DEMO_MYHANDLE_H
#define CPP_TCP_DEMO_MYHANDLE_H

#include <list>
#include <mutex>
#include <map>
#include "Handler.h"
#include "RWLock.hpp"
#include <thread>
#include "libs/my_socket.h"
#include <cstdio>
#include <unistd.h>
#include <cstring>
#include <iostream>
#include "./libs/http/HttpServer.h"
using namespace std;
#define MAXLINE 4096
#define CLIENT_PORT 8528
#define HTTP_PORT 8529
struct ServerInfo{
    ServerInfo(string _ip, int _proportion):ip(_ip), proportion(_proportion){}
    string ip;
    int proportion;
};


class MyHandle : public Handler {

public:
    MyHandle(string username, string pwd);
    ~MyHandle();
    void Server(int connfd, string remoteIp);
    void HeartCheck(int);
    void HeartCheckEntry();
    void PreCheck();
    bool DoCheck(const string &ip);
    void DeleteAddr(string ip);
    void HttpLogin(Request req, Response *resp);
    void HttpAddServer(Request req, Response *resp);
    void HttpDelServer(Request req, Response *resp);
    void HttpGetAllServer(Request req, Response *resp);
    void HttpChangeServer(Request req, Response *resp);

private:
    typedef map<string, list<ServerInfo *> *> Server_map;
    string http_username;
    string http_pwd;
    RWLock lock;
    int heart_check_time;
    HttpServer *http_server;
    Server_map server_list_map;
};

bool count(const list<string> &l, string target);
bool count(const list<ServerInfo*> &l, string target);

#endif //CPP_TCP_DEMO_MYHANDLE_H
