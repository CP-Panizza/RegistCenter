//
// Created by cmj on 20-3-26.
//

#include "Response.h"
#include "util.h"
#include <string>
#include <iostream>
#include <sys/socket.h>
#include <cstring>
#include <fstream>
#include <sstream>

void Response::set_header(std::string key, std::string val) {
    header[key] = val;
}


void Response::write(int code, std::string data) {
    std::string buf = "HTTP/1.1 " + std::to_string(code) + " " + get_descript(code) + "\r\n";
    for (auto x : header) {
        buf += x.first + ":" + x.second + "\r\n";
    }
    buf += "\r\n";
    buf += data;
    send(conn, buf.c_str(), strlen(buf.c_str()), 0);
}

std::string Response::get_descript(int code) {
    std::string str;
    switch (code) {
        case 100:
            str = "Continue";
            break;//(继续)	收到了请求的起始部分，客户端应该继续请求
        case 101:
            str = "Switching Protocols ";
            break;//(切换协议)	服务器正根据客户端的指示将协议切换成Update，首部列出的协议
        case 200:
            str = "OK";
            break;//服务器已成功处理请求
        case 201:
            str = "Created";
            break;//(已创建)	对那些要服务器创建对象的请求来说，资源已创建完毕
        case 202:
            str = "Accepted";
            break;//(已接受)	请求已接受，但服务器尚未处理
        case 203:
            str = "Non-Authritative";
            break;//(非权威信息)	服务器已将事物成功处理，知识实体首部包含的信息不是来自原始服务器，而是来自资源的副本
        case 204:
            str = "No Content";
            break;//(没有内容)	响应报文包含一些首部和一个状态行，但不包含实体的主体内容
        case 205:
            str = "Reset Content";
            break;//(重置内容)	另一个主要用于浏览器的代码。意思是浏览器应该重置当前页面上所有的HTML表单
        case 206:
            str = "Partial Content";
            break;//(部分内容)	部分请求成功
        case 300:
            str = "Multiple Choices";
            break;//(多项选择)	客户端请求了实际指向多个资源的URL。这个代码是和一个选项列表一起返回的，然后用户就可以选择他希望使用的选项了
        case 301:
            str = "Moved Permanently";
            break;//(永久移除)	请求的URL已移走。响应中应该包含一个Location URL，说明资源现在所处的位置
        case 302:
            str = "Found";
            break;//(已找到)	与状态码301类似，但这里的移除是临时的。客户端应该用Location，首部给出的URL对资源进行临时定位
        case 303:
            str = "See Other";
            break;//(参见其他)	告诉客户端应该用另一个URL获取资源。这个新的URL位于响应报文的Location首部
        case 304:
            str = "Not Modified";
            break;//(未修改)	客户端可以通过它们所包含的请求首部发起条件请求。这个代码说明资源未发生过变化
        case 305:
            str = "Use Proxy";
            break;//(使用代理)	必须通过代理访问资源，代理的位置是在Location首部中给出的
        case 307:
            str = "Temporary Redirect";
            break;//(临时重定向)	和状态码301类似。但客户端应该用Location首部给出的URL对资源进行临时定位
        case 400:
            str = "Bad request";
            break;//(坏请求)	告诉客户端它发送了一条异常请求
        case 401:
            str = "Unauthorized";
            break;//(未授权)	与适当的首部一起返回，在客户端获得资源访问权之前，请它进行身份认证
        case 402:
            str = "Payment Required";
            break;//(要求付款)	当前此状态码并未使用，是为未来使用预留的
        case 403:
            str = "Forbidden";
            break;//(禁止)	服务器拒绝了请求
        case 404:
            str = "Not Found";
            break;//(未找到)	服务器无法找到所请求的URL
        case 405:
            str = "Method Not Allowed";
            break;//(不允许使用的方法)	请求中有一个所请求的URI不支持的方法。响应中应该包含一个Allow首部，以告知客户端所请求的资源支持使用哪些方法
        case 406:
            str = "Not Acceptable";
            break;//(无法接受)	客户端可以指定一些参数来说明希望接受哪些类型的实体。服务器没有资源与客户端可接受的URL相匹配时可使用此代码
        case 407:
            str = "Proxy Authentication";
            break;//(要求进行代理认证)	和状态码401类似，但用于需要进行资源认证的代理服务器
        case 408:
            str = "Request Timeout";
            break;//(请求超时)	如果客户端完成请求时花费的时间太长，服务器可以回送这个状态码并关闭连接
        case 409:
            str = "Confict";
            break;//(冲突)	发出的请求在资源上造成了一些冲突
        case 410:
            str = "Gone";
            break;//(消失了)	除了服务器曾持有这些资源之外，与状态码404类似
        case 411:
            str = "Length Required";
            break;//(要求长度指示)	服务器要求在请求报文中包含Content-Length首部时会使用这个代码。发起的请求中若没有Content-Length首部，服务器是不会接受此资源请求的
        case 412:
            str = "Precondtion Failed";
            break;//(先决条件失败)	如果客户端发起了一个条件请求，如果服务器无法满足其中的某个条件，就返回这个响应码
        case 413:
            str = "Request Enity Too large";
            break;//(请求实体太大)	客户端发送的实体主体部分比服务器能够或者希望处理的要大
        case 414:
            str = "Request URI Too Long";
            break;//(请求URI太长)	客户端发送的请求所携带的请求URL超过了服务器能够活着希望处理的长度
        case 415:
            str = "Unsupported Media Type";
            break;//(不支持的媒体类型)	服务器无法理解或不支持客户端所发送的实体的内容类型
        case 416:
            str = "Requested Range Not Satisfiable";
            break;//	请求报文请求的是某范围内的指定资源，但那个范围无效，或者未得到满足
        case 417:
            str = "Expectation Failed";
            break;//(无法满足期望)	请求的Expect首部包含了一个预期的内容，但服务器无法满足
        case 500:
            str = "internal Server Error";
            break;//(内部服务器错误)	服务器遇到一个错误，使其无法为请求提供服务
        case 501:
            str = "Not Implemented";
            break;//(未实现)	服务器无法满足客户端请求的某个功能
        case 502:
            str = "Bad Gateway";
            break;//(网关故障)	作为代理或网关使用的服务器遇到了来自响应链中上游的无效响应
        case 503:
            str = "Service Unavailable";
            break;//(未提供此服务)	服务器目前无法为请求提供服务，但过一段时间就可以恢复服务
        case 504:
            str = "Gateway Timeout";
            break;//(网关超时)	与状态码408类似，但是响应来自网关或代理，此网关或代理在等待另一台服务器的响应时出现了超时
        case 505:
            str = "HTTP Version Not Supported";
            break;//(不支持的HTTP版本)
        default:
            str = "Unkonw Code";
    }
    return str;
}


void Response::send_file(std::string path) {
    std::string buf = "HTTP/1.1 " + std::to_string(200) + " " + get_descript(200) + "\r\n";
    buf += ("Content-Type:" + get_file_type(path) + "\r\n");
    buf += ("Content-Length:" + std::to_string(file_size(path.c_str()))  + "\r\n");
    buf += "\r\n";
    if (send(conn, (void *) buf.c_str(), strlen(buf.c_str()), 0) < 0) {
        printf("send head error: %s(errno: %d)\n", strerror(errno), errno);
    }

    int fd = open(path.c_str(), O_RDONLY);
    if(fd == -1){
        perror("open err");
        return;
    }
    char buff[4096];
    int ret = 0;
    while((ret = read(fd, buff, sizeof(buff))) > 0){
        send(conn, buff, ret, 0);
    }
    if(ret == -1){
        perror("read err");
        return;
    }
    close(fd);
}

std::string Response::get_file_type(std::string file) {
    auto type = split(file, ".");
    if (type.size() != 2) {
        return "text/plain;charset=utf-8";
    }

    if (type[1] == "html")
        return "text/html; charset=utf-8";
    if (type[1] == "jpg")
        return "image/jpeg";
    if (type[1] == "gif")
        return "image/gif";
    if (type[1] == "png")
        return "image/png";
    if (type[1] == "css")
        return "text/css";
    if (type[1] == "au")
        return "audio/basic";
    if (type[1] == "wav")
        return "audio/wav";
    if (type[1] == "avi")
        return "video/x-msvideo";
    if (type[1] == "mov")
        return "video/quicktime";
    if (type[1] == "mpeg")
        return "video/mpeg";
    if (type[1] == "vrml")
        return "model/vrml";
    if (type[1] == "midi")
        return "audio/midi";
    if (type[1] == "mp3")
        return "audio/mpeg";
    if (type[1] == "ogg")
        return "application/ogg";
    if (type[1] == "pac")
        return "application/x-ns-proxy-autoconfig";

    return "";
}
