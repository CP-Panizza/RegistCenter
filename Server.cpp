//
// Created by Administrator on 2020/3/6.
//


#include "Server.h"
#include "libs/my_socket.h"
#include <iostream>


#ifdef _WIN64
#include <ws2tcpip.h>
#else
#include <netinet/in.h>
#include<sys/socket.h>
#include <arpa/inet.h>
#endif

Server::Server(int port) {
#ifdef _WIN64
    WORD dwVersion = MAKEWORD(2, 2);
    WSAData wsaData;
    WSAStartup(dwVersion, &wsaData);
    sockaddr_in servaddr;
    memset(&servaddr, 0, sizeof(servaddr));
    servaddr.sin_family = AF_INET; //网络类型
    servaddr.sin_addr.s_addr = htonl(INADDR_ANY);
    servaddr.sin_port = htons(port); //端口
    socket_fd = 0;

    if ((socket_fd = socket(AF_INET, SOCK_STREAM, 0)) == INVALID_SOCKET) {
        printf("create socket error: %s(errno: %d)\n", strerror(errno), errno);
        WSACleanup();
        exit(-1);
    }

    bool bReAddr = true;
    if (SOCKET_ERROR == (setsockopt(socket_fd, SOL_SOCKET, SO_REUSEADDR, (char *) &bReAddr, sizeof(bReAddr)))) {
        std::cout << "set resueaddr socket err!" << std::endl;
        WSACleanup();
        exit(-1);
    }

    if (bind(socket_fd, (struct sockaddr *) &servaddr, sizeof(servaddr)) == INVALID_SOCKET) {
        printf("bind socket error: %s(errno: %d)\n", strerror(errno), errno);
        WSACleanup();
        exit(-1);
    }

    //监听，设置最大连接数10
    if (listen(socket_fd, 10) == INVALID_SOCKET) {
        printf("listen socket error: %s(errno: %d)\n", strerror(errno), errno);
        WSACleanup();
        exit(-1);
    }
    FD_ZERO(&select_fd);
    FD_SET(socket_fd, &select_fd);
#else
    struct sockaddr_in servaddr;
    memset(&servaddr, 0, sizeof(servaddr));
    servaddr.sin_family = AF_INET; //网络类型
    servaddr.sin_addr.s_addr = htonl(INADDR_ANY);
    servaddr.sin_port = htons(port); //端口
    socket_fd = 0;

    if ((socket_fd = socket(AF_INET, SOCK_STREAM, 0)) == -1) {
        printf("create socket error: %s(errno: %d)\n", strerror(errno), errno);
        exit(-1);
    }

    if (bind(socket_fd, (struct sockaddr *) &servaddr, sizeof(servaddr)) == -1) {
        printf("bind socket error: %s(errno: %d)\n", strerror(errno), errno);
        exit(-1);
    }

    //监听，设置最大连接数10
    if (listen(socket_fd, 10) == -1) {
        printf("listen socket error: %s(errno: %d)\n", strerror(errno), errno);
        exit(-1);
    }

    epoll_events = new struct epoll_event[MAX_COUNT];

    epoll_fd = epoll_create(MAX_COUNT);
    struct epoll_event ev;
    ev.data.fd = socket_fd;
    ev.events = EPOLLIN;
    epoll_ctl(epoll_fd, EPOLL_CTL_ADD, socket_fd, &ev);
#endif
}



void Server::Start(Handler *handler) {
    printf("======waiting for client's request======\n");
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
            //获取到套接字
            SOCKET s = temp_fd.fd_array[i];
            if (FD_ISSET(s, &temp_fd)) {
                //接收到客户端的链接
                if (s == socket_fd) {
                    do_accept();
                } else {
                    handler->Server(s, clients[s]);
                    disconnect(s);
                }
            }
        }
    }
#else
    int ret;
    while (true) {
        ret = epoll_wait(epoll_fd, epoll_events, MAX_COUNT, -1);
        if (ret <= 0) {
            continue;
        } else {
            for (int i = 0; i < ret; i++) {
                if (epoll_events[i].data.fd == socket_fd) {
                    do_accept();
                } else {
                    handler->Server(epoll_events[i].data.fd, clients[epoll_events[i].data.fd]);
                    disconnect(epoll_events[i].data.fd);
                }
            }
        }
    }
#endif
}


void Server::do_accept() {
#ifdef _WIN64
    struct sockaddr_in addrClient;
    socklen_t len = sizeof(struct sockaddr_in);
    SOCKET c = accept(socket_fd, (struct sockaddr *) &addrClient, &len);
    if (c == INVALID_SOCKET) {
        std::cout << "accept err!" << std::endl;
        return;
    }
    clients[c] = inet_ntoa(addrClient.sin_addr);
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
    clients[new_fd] = inet_ntoa(cli.sin_addr);
#endif
}

Server::~Server() {

}


#ifdef _WIN64
void Server::disconnect(int cfd) {
    clients.erase(cfd);
    FD_CLR(cfd, &select_fd);
    closesocket(cfd);
}
#else
void Server::disconnect(int cfd) {
    clients.erase(cfd);
    int ret = epoll_ctl(epoll_fd, EPOLL_CTL_DEL, cfd, NULL);
    if (ret == -1) {
        perror("epoll_ctl del cfd error");
        return;
    }
    close(cfd);
}
#endif



