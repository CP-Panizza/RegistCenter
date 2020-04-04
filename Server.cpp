//
// Created by Administrator on 2020/3/6.
//


#include "Server.h"
#include "libs/my_socket.h"
#include <c++/iostream>
#include <ws2tcpip.h>


Server::Server(int port) {

    WORD dwVersion = MAKEWORD(2, 2);
    WSAData wsaData;
    WSAStartup(dwVersion, &wsaData);

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
}


Server::~Server() {
}

void Server::Start(Handler *handler) {
    printf("======waiting for client's request======\n");
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
}


void Server::do_accept() {
    struct sockaddr_in addrClient;
    socklen_t len = sizeof(struct sockaddr_in);
    SOCKET c = accept(socket_fd, (struct sockaddr *) &addrClient, &len);
    if (c == INVALID_SOCKET) {
        std::cout << "accept err!" << std::endl;
        return;
    }
    clients[c] = inet_ntoa(addrClient.sin_addr);
    FD_SET(c, &select_fd);
}


void Server::disconnect(int cfd) {
    clients.erase(cfd);
    FD_CLR(cfd, &select_fd);
    closesocket(cfd);
}



