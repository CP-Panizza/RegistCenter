//
// Created by Administrator on 2020/4/1.
//

#ifndef HTTP_WINDOWS_HTTPSERVER_H
#define HTTP_WINDOWS_HTTPSERVER_H
#include <functional>
#include <map>
#include <list>
#include "../my_socket.h"
#include "Request.h"
#include "Response.h"

#ifndef _WIN64
#include  <sys/epoll.h>
#ifndef _MAX_COUNT_
#define MAX_COUNT 1024
#endif
#endif
struct handle{
    handle(std::string u, std::function<void(Request, Response *)> m):url(u), method(m){}
    std::string url;
    std::function<void(Request, Response *)> method;
};

class HttpServer {
public:
    HttpServer(int port);
    ~HttpServer();
    void set_static_path(std::string);

    void do_accept();

    void disconnect(int cfd);
    void H(std::string method, std::string url, std::function<void(Request, Response*)> func);
    void Thread_handle(int conn);
    void run();

private:
    std::string excute_pwd; //程序启动路径
    std::string static_path;
#ifdef _WIN64
    SOCKET socket_fd;
    fd_set select_fd;
#else
    int socket_fd;
    struct epoll_event *epoll_events;
    int epoll_fd;
#endif
    int m_port;
    std::map<std::string, std::list<handle*>*> methods;
};


#endif //HTTP_WINDOWS_HTTPSERVER_H
