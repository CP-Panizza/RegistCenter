//
// Created by Administrator on 2020/4/1.
//



#include "HttpServer.h"
#include "util.h"
#include <stdlib.h>
#include <iostream>
#include "Request.h"
#include "Response.h"

#ifdef _WIN64
#include <inaddr.h>
#include <ws2tcpip.h>
#else

#include <sys/socket.h>
#include <netinet/in.h>
#include <cstring>

#endif

#define MAXLINE 4096

HttpServer::HttpServer(int port) : m_port(port) {
    char buf[100] = {0};
    getcwd(buf, sizeof(buf));
    this->excute_pwd = std::string(buf);

#ifdef _WIN64
    WORD dwVersion = MAKEWORD(2, 2);
    WSAData wsaData;
    WSAStartup(dwVersion, &wsaData);
#endif


    socket_fd = socket(AF_INET, SOCK_STREAM, 0);
#ifdef _WIN64
    if (socket_fd == INVALID_SOCKET) {
        std::cout << "create socket err!" << std::endl;
        WSACleanup();
        exit(-1);
    }
#else
    if (socket_fd == -1) {
        std::cout << "create socket err!" << std::endl;
        exit(-1);
    }
#endif

    sockaddr_in addr;
    memset(&addr, 0, sizeof(sockaddr_in));
    addr.sin_family = AF_INET;
    addr.sin_port = htons(port);
#ifdef _WIN64
    addr.sin_addr.S_un.S_addr = htonl(INADDR_ANY);
#else
    addr.sin_addr.s_addr = htonl(INADDR_ANY);
#endif

#ifdef _WIN64
    bool bReAddr = true;
    if (SOCKET_ERROR == (setsockopt(socket_fd, SOL_SOCKET, SO_REUSEADDR, (char *) &bReAddr, sizeof(bReAddr)))) {
        std::cout << "set resueaddr socket err!" << std::endl;
        WSACleanup();
        exit(-1);
    }
#endif

#ifdef _WIN64
    if ((bind(socket_fd, (struct sockaddr *) &addr, sizeof(addr))) == SOCKET_ERROR) {
        std::cout << "bind socket err!" << std::endl;
        WSACleanup();
        exit(-1);
    }
#else
    if ((bind(socket_fd, (struct sockaddr *) &addr, sizeof(addr))) == -1) {
        std::cout << "bind socket err!" << std::endl;
        exit(-1);
    }
#endif


#ifdef _WIN64
    if (listen(socket_fd, 5) != 0) {
        std::cout << "listen socket err!" << std::endl;
        WSACleanup();
        exit(-1);
    }

    FD_ZERO(&select_fd);
    FD_SET(socket_fd, &select_fd);
#else
    if (listen(socket_fd, 5) != 0) {
        std::cout << "listen socket err!" << std::endl;
        exit(-1);
    }

    //初始化事件列表
    epoll_events = new struct epoll_event[100];
    epoll_fd = epoll_create(MAX_COUNT);
    struct epoll_event ev;
    ev.data.fd = socket_fd;
    ev.events = EPOLLIN;
    epoll_ctl(epoll_fd, EPOLL_CTL_ADD, socket_fd, &ev);
#endif
}


void HttpServer::run() {
    std::cout << "http server started at port:" << std::to_string(m_port) << std::endl;
#ifdef _WIN64
    int ret;
    fd_set temp_fd;
    struct timeval t = {5, 0};
    while (true) {
        FD_ZERO(&temp_fd);
        temp_fd = select_fd;

        ret = select(0, &temp_fd, NULL, NULL, &t);//最后一个参数为NULL，一直等待，直到有数据过来
        if (ret == SOCKET_ERROR) {
            printf("%s", "select err!");
            continue;
        }

        for (int i = 0; i < temp_fd.fd_count; ++i) {
            if (FD_ISSET(temp_fd.fd_array[i], &temp_fd)) {
                //接收到客户端的链接
                if (temp_fd.fd_array[i] == socket_fd) {
                    do_accept();
                } else {
                    Thread_handle(temp_fd.fd_array[i]);
                }
            }
        }
    }
#else
    int ret;
    while (1) {
        ret = epoll_wait(epoll_fd, epoll_events, MAX_COUNT, -1);
        if (ret <= 0) {
            continue;
        } else {
            for (int i = 0; i < ret; i++) {
                if (epoll_events[i].data.fd == socket_fd) {
                    do_accept();
                } else {
                    Thread_handle(epoll_events[i].data.fd);
                }
            }
        }
    }
#endif
}

#ifdef _WIN64
// 断开连接的函数,并从事件数组中删除此连接
void HttpServer::disconnect(int cfd) {
    FD_CLR(cfd, &select_fd);
    closesocket(cfd);

}
#else
void HttpServer::disconnect(int cfd) {
    int ret = epoll_ctl(epoll_fd, EPOLL_CTL_DEL, cfd, NULL);
    if (ret == -1) {
        perror("epoll_ctl del cfd error");
        return;
    }
    close(cfd);
}
#endif

#ifdef _WIN64
void HttpServer::Thread_handle(int conn) {
    Request request;
    char buf[MAXLINE] = {0};
    int n;
    n = recv(conn, buf, MAXLINE, 0);
    if (n == SOCKET_ERROR || n == 0) {
        disconnect(conn);
        return;
    }
    buf[n] = '\0';
    try {
        request.Paser(std::string(buf));
    } catch (std::string err) {
        std::cout << err << std::endl;
        disconnect(conn);
        return;
    }

    std::cout << "DEBUG [" << request.method << "-----" << request.url << "]" << std::endl;
    if (request.header.count("Content-Length")) {
        long content_length = atol(request.header["Content-Length"].c_str());
        while (request.body.size() < content_length) {
            char buff[MAXLINE];
            int len;
            len = recv(conn, buff, MAXLINE, 0);
            if (len < 0) {
                disconnect(conn);
                return;
            }
            buff[len] = '\0';
            request.body += std::string(buff);
        }
    }

    Response response(conn);
    if (request.method == "GET" && excute_pwd != "" && this->static_path != "") {
        std::string dir = excute_pwd + static_path + request.path;
        dir = replace_all(dir, "/", "\\");
        std::string index = dir + "\\index.html";
        if (file_exists(index)) {
            response.send_file(index);
            disconnect(conn);
            return;
        } else if (file_exists(dir)) {
            response.send_file(dir);
            disconnect(conn);
            return;
        }
    }

    bool ok = false;
    std::function<void(Request, Response *)> temp_handle;
    if (methods.count(request.method)) {
        auto l = methods[request.method];
        if (l->size() != 0) {
            for (auto h : *l) {
                if (h->url == request.path) {
                    temp_handle = h->method;
                    ok = true;
                    break;
                }
            }
        }
    }

    if (ok) {
        temp_handle(request, &response);
    } else {
        response.set_header("Content-Type", "text/html");
        response.write(404, "Not found!");
        disconnect(conn);
    }

    disconnect(conn);
}
#else
void HttpServer::Thread_handle(int conn) {
    Request request;
    char buf[MAXLINE] = {0};
    int n;
    n = recv(conn, buf, MAXLINE, 0);
    if (n < 0 || n == 0) {
        disconnect(conn);
        return;
    }
    buf[n] = '\0';
    try {
        request.Paser(std::string(buf));
    } catch (std::string err) {
        std::cout << err << std::endl;
        disconnect(conn);
        return;
    }

    if(request.header.count("Content-Length")){
        long content_length = atol(request.header["Content-Length"].c_str());
        if(request.body.size() < content_length){
            char buff[MAXLINE];
            int len;
            len = recv(conn, buff, MAXLINE, 0);
            if (len < 0) {
                disconnect(conn);
                return;
            }
            buff[len] = '\0';
            request.body = std::string(buff);
        }
    }

    Response response(conn);
    if (request.method == "GET" && excute_pwd != "" && this->static_path != "") {
        std::string dir = excute_pwd + static_path + request.path;
        auto v = split(dir, "/");
        std::string Dir;
        for (int i = 0; i < v.size(); ++i) {
            if (v[i] != "") {
                Dir += "/" + v[i];
            }
        }
        std::string index = Dir + "/index.html";
        if (file_exists(index)) {
            response.send_file(index);
            disconnect(conn);
            return;
        } else if (file_exists(Dir)) {
            response.send_file(Dir);
            disconnect(conn);
            return;
        }
    }

    bool ok = false;
    std::function<void(Request, Response *)> temp_handle;
    if (methods.count(request.method)) {
        auto l = methods[request.method];
        if (l->size() != 0) {
            for (auto h : *l) {
                if (h->url == request.path) {
                    temp_handle = h->method;
                    ok = true;
                    break;
                }
            }
        }
    }

    if (ok) {
        temp_handle(request, &response);
    } else {
        response.set_header("Content-Type", "text/html");
        response.write(404, "Not found!");
    }

    disconnect(conn);
}
#endif


void HttpServer::H(std::string method, std::string url, std::function<void(Request, Response *)> func) {
    auto h = new struct handle(url, func);
    if (methods.count(method)) {
        methods[method]->push_back(h);
    } else {
        auto temp = new std::list<handle *>;
        temp->push_back(h);
        methods[method] = temp;
    }
}


HttpServer::~HttpServer() {
#ifdef _WIN64
    //关闭监听套接字
    closesocket(socket_fd);
    //清理winsock环境
    WSACleanup();
#endif
}


void HttpServer::do_accept() {
#ifdef _WIN64
    SOCKET c = accept(socket_fd, NULL, NULL);
    if (c == INVALID_SOCKET) {
        std::cout << "accept err!" << std::endl;
        return;
    }
    FD_SET(c, &select_fd);
#else
    struct sockaddr_in cli;
    socklen_t len = sizeof(cli);
    int new_fd = accept(socket_fd, (struct sockaddr *) &cli, &len);
    if (new_fd == -1) {
        perror("accept err");
        return;
    }
    struct epoll_event ev;
    ev.events = EPOLLIN;
    ev.data.fd = new_fd;
    int ret = epoll_ctl(epoll_fd, EPOLL_CTL_ADD, new_fd, &ev);
    if (ret == -1) {
        perror("epoll_ctl err");
        return;
    }
#endif
}




#ifdef _WIN64
void HttpServer::set_static_path(std::string path) {
    if (path[0] != '\\') {
        throw std::string("STATIC PATH MUST START WITH '/'");
    }
    this->static_path = path;
}
#else
void HttpServer::set_static_path(std::string path) {
    if (path[0] != '/') {
        throw std::string("STATIC PATH MUST START WITH '/'");
    }
    this->static_path = path;
}
#endif