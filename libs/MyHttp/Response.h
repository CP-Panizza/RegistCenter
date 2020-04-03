//
// Created by cmj on 20-3-26.
//

#ifndef HTTP_RESPONSE_H
#define HTTP_RESPONSE_H
#include <map>
#include <fcntl.h>
#include <unistd.h>
#include <sys/socket.h>


class Response {
private:
    int conn;
    std::map<std::string,std::string> header;
    std::string get_descript(int code);

public:
    Response(int fd):conn(fd){}
    void set_header(std::string, std::string);
    void write(int,std::string);
    void send_file(std::string path);
    std::string get_file_type(std::string file);
};


#endif //HTTP_RESPONSE_H
