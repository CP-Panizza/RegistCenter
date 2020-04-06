//
// Created by cmj on 20-3-26.
//

#ifndef HTTP_RESPONSE_H
#define HTTP_RESPONSE_H
#include <map>
#include <unistd.h>
#include "../my_socket.h"


class Response {
private:
#ifdef _WIN64
    SOCKET conn;
#else
    int conn;
#endif
    std::map<std::string,std::string> header;
    std::string get_descript(int code);

public:
#ifdef _WIN64
    Response(SOCKET fd):conn(fd){}
#else
    Response(int fd):conn(fd){}
#endif
    void set_header(std::string, std::string);
    void write(int,std::string);
    void send_file(std::string path);
    std::string get_file_type(std::string file);
};


#endif //HTTP_RESPONSE_H
