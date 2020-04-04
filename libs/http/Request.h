//
// Created by cmj on 20-3-26.
//

#ifndef HTTP_REQUEST_H
#define HTTP_REQUEST_H
#include <string>
#include<map>

class Request {
public:
    std::string method;
    std::string http_version;
    std::string url;
    std::string path;
    std::map<std::string, std::string> params;
    std::map<std::string, std::string> header;
    std::string body;

    void Paser(std::string data);
};



#endif //HTTP_REQUEST_H
