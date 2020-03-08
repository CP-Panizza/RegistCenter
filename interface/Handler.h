//
// Created by Administrator on 2020/3/6.
//

#ifndef CPP_TCP_DEMO_HANDLER_H
#define CPP_TCP_DEMO_HANDLER_H
#include <string>

class Handler {
public:
    Handler(){};
    virtual ~Handler(){};
    virtual void Server(int connfd, std::string remoteIp, int port) = 0;
};


#endif //CPP_TCP_DEMO_HANDLER_H
