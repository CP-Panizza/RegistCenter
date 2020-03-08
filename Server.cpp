//
// Created by Administrator on 2020/3/6.
//


#include "Server.h"
#include <sys/ioctl.h>

#include <arpa/inet.h>

Server::Server(int port) {
    memset(&servaddr, 0, sizeof(servaddr));
    servaddr.sin_family = AF_INET; //网络类型
    servaddr.sin_addr.s_addr = htonl(INADDR_ANY);
    servaddr.sin_port = htons(port); //端口
    listenfd = 0;
}


Server::~Server() {
}



void Server::Init() {
    if ((listenfd = socket(AF_INET, SOCK_STREAM, 0)) == -1) {
        printf("create socket error: %s(errno: %d)\n", strerror(errno), errno);
        exit(-1);
    }



    if (bind(listenfd, (struct sockaddr *) &servaddr, sizeof(servaddr)) == -1) {
        printf("bind socket error: %s(errno: %d)\n", strerror(errno), errno);
        exit(-1);
    }


    //监听，设置最大连接数10
    if (listen(listenfd, 10) == -1) {
        printf("listen socket error: %s(errno: %d)\n", strerror(errno), errno);
        exit(-1);
    }


}


void Server::Start(Handler *handler) {
    printf("======waiting for client's request======\n");
    while (1) {
        int connfd;
        struct sockaddr_in addrClient;
        socklen_t len = sizeof(struct sockaddr_in);
        if ((connfd = accept(listenfd, (struct sockaddr *)&addrClient, &len)) == -1) {
            printf("accept socket error: %s(errno: %d)", strerror(errno), errno);
            continue;
        }


        thread t(&Handler::Server, handler, connfd, string(inet_ntoa(addrClient.sin_addr)), (int)ntohs(addrClient.sin_port));
        t.detach();
    }
}


void Server::Close() {
    printf("%s\n", "close listen!");
    close(listenfd);
}

