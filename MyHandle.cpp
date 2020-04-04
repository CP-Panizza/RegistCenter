//
// Created by Administrator on 2020/3/6.
//


#include<cstdio>
#include<cstdlib>
#include<cstring>
#include<cerrno>
#include<sys/types.h>
#include <winsock2.h>
#include<unistd.h>
#include "MyHandle.h"
#include "libs/http/Response.h"
#include "libs/http/Request.h"
#include "libs/http/HttpServer.h"
#include "libs/rapidjson/document.h"
#include "libs/rapidjson/writer.h"
#include "libs/rapidjson/stringbuffer.h"

using namespace rapidjson;
using namespace std;

MyHandle::MyHandle(string username, string pwd):http_username(username), http_pwd(pwd) {
    http_server = new HttpServer(HTTP_PORT);
    http_server->set_static_path("\\resource");
    auto login = std::bind(&MyHandle::HttpLogin, this, std::placeholders::_1,std::placeholders::_2);
    auto get_all = std::bind(&MyHandle::HttpGetAllServer, this, std::placeholders::_1,std::placeholders::_2);
    auto del_server = std::bind(&MyHandle::HttpDelServer, this, std::placeholders::_1,std::placeholders::_2);
    auto add_server = std::bind(&MyHandle::HttpAddServer, this, std::placeholders::_1,std::placeholders::_2);
    http_server->H("POST", "/login", login);
    http_server->H("GET", "/getAll", get_all);
    http_server->H("DELETE", "/del", del_server);
    http_server->H("POST", "/add", add_server);
    thread t(&HttpServer::run,http_server);
    t.detach();
}

MyHandle::~MyHandle() {

}


/**
 * {
 *  Op: "REG|PULL",
 *  ServiceList : ["User.Login", "User.Delete"],
 *  ServicePort : ":8080"
 * }
 *
 * @param connfd
 * @param remoteIp
 * @param port
 */
void MyHandle::Server(int connfd, std::string remoteIp) {
    char buff[MAXLINE];
    string respmsg;
    int n;
    n = recv(connfd, buff, MAXLINE, 0);
    buff[n] = '\0';
    printf("recv msg from client: %s\n", buff);

    Document d;
    if (d.Parse(buff).HasParseError()) {
        respmsg = "{\"ok\":false, \"msg\":\"Data Err\", \"data\":[]}";
    } else if (d.HasMember("Op") && string(d["Op"].GetString()) == "REG" && d.HasMember("ServiceList") &&
               d.HasMember("ServicePort")) { //注册服务
        //解析客户端发送过来的服务信息数据
        string port_str(d["ServicePort"].GetString());
        remoteIp.append(port_str);
        int proportion = d["Proportion"].GetInt();
        const Value &serverList = d["ServiceList"];
        for (int i = 0; i < serverList.Size(); ++i) {
            string serName(serverList[i].GetString());
            lock.lockWrite();
            if (server_list_map.count(serName)) {
                list<ServerInfo> *temp = server_list_map[serName];
                temp->push_front(ServerInfo{remoteIp, proportion});
            } else {
                auto *templist = new list<ServerInfo>;
                templist->push_front(ServerInfo{remoteIp, proportion});
                server_list_map[serName] = templist;
            }
            lock.unlockWrite();
        }
        respmsg = "{\"ok\":true, \"msg\":\"Done\", \"data\":[]}";

        /**
         * PULL:Data{
         *  Op:"PULL",
         *  ServiceList : ["User.Login", "User.Delete"]
         * }
         */
    } else if (d.HasMember("Op") && string(d["Op"].GetString()) == "PULL" && d.HasMember("ServiceList")) { //拉取服务列表
        rapidjson::StringBuffer s;
        rapidjson::Writer<rapidjson::StringBuffer> writer(s);

        map<string, list<ServerInfo> *> temp_map;
        auto serviceList = d["ServiceList"].GetArray();

        for (auto it = serviceList.Begin(); it != serviceList.End(); it++) {
            string ser_name(it->GetString());
            lock.lockRead();
            if (server_list_map.count(ser_name)) {
                temp_map[ser_name] = server_list_map[ser_name];
            } else {
                list<ServerInfo> tmep_list;
                temp_map[ser_name] = &tmep_list;
            }
            lock.unlockRead();
        }
        writer.StartObject();
        writer.Key("ok");
        writer.Bool(true);
        writer.Key("msg");
        writer.String("Done");

        writer.Key("data");
        writer.StartArray();
        for (auto key_val : temp_map) {
            writer.StartObject();
            writer.Key(key_val.first.c_str());
            writer.StartArray();
            for (auto &ip : *key_val.second) {
                writer.StartObject();
                writer.Key("Ip");
                writer.String(ip.ip.c_str());
                writer.Key("Proportion");
                writer.Int(ip.proportion);
                writer.EndObject();
            }
            writer.EndArray();
            writer.EndObject();
        }
        writer.EndArray();
        writer.EndObject();
        respmsg = string(s.GetString());
    }

    if (send(connfd, respmsg.c_str(), strlen(respmsg.c_str()), 0) < 0) {
        printf("send msg error: %s(errno: %d)\n", strerror(errno), errno);
    }
}


/**
 * 轮训检测服务端是否在线,time秒
 */
void MyHandle::HeartCheck(int time) {
    this->heart_check_time = time;

    cout << "start heartCheck" << endl;
    thread t(&MyHandle::HeartCheckEntry, this);
    t.detach();
}


void MyHandle::HeartCheckEntry() {
    while (true) {
        if (server_list_map.empty()) {
            this_thread::sleep_for(std::chrono::milliseconds(5000));
            continue;
        }

        PreCheck();
        this_thread::sleep_for(std::chrono::seconds(heart_check_time));
    }
}


bool count(const list<string> &l, string target) {
    for (auto x : l) {
        if (x == target)
            return true;
    }
    return false;
}


bool count(const list<ServerInfo> &l, string target) {
    for (auto x : l) {
        if (x.ip == target)
            return true;
    }
    return false;
}


bool MyHandle::DoCheck(const string &ip) {
    SOCKET sockfd, n;
    char recvline[100], sendline[] = "CHECK";
    struct sockaddr_in servaddr;
    if ((sockfd = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
        printf("create socket error: %s(errno: %d)\n", strerror(errno), errno);
        return true;
    }
    memset(&servaddr, 0, sizeof(servaddr));
    servaddr.sin_family = AF_INET;
    servaddr.sin_port = htons(CLIENT_PORT);
    servaddr.sin_addr.s_addr = inet_addr(ip.c_str());


    if (connect(sockfd, (struct sockaddr *) &servaddr, sizeof(servaddr)) < 0) {
        printf("connect error: %s(errno: %d)\n", strerror(errno), errno);
        return false;
    }

    if (send(sockfd, sendline, strlen(sendline), 0) < 0) {
        printf("send msg error: %s(errno: %d)\n", strerror(errno), errno);
        return false;
    }

    if ((n = recv(sockfd, recvline, sizeof(recvline), 0)) < 0) {
        printf("%s\n", "recv err");
        return false;
    }
    recvline[n] = '\0';
    close(sockfd);
    return true;
}

/**
 * 删除拼不通的远端服务ip
 * @param ip
 */
void MyHandle::DeleteAddr(string ip) {
    list<string> emptyListName;

    for (auto x : server_list_map) {
        if (count(*(x.second), ip)) {
            x.second->remove_if([=](ServerInfo n) { return n.ip == ip; });
            if (x.second->empty()) {
                emptyListName.push_back(x.first);
            }
        }
    }

    for (auto x : emptyListName) {
        server_list_map.erase(x);
    }
}


void MyHandle::PreCheck() {
    list<string> addrs; //存放去重复后的远端服务器地址
    this->lock.lockRead();
    for (auto l : server_list_map) {
        for (auto ip : *(l.second)) {
            if (!count(addrs, ip.ip)) {
                addrs.push_front(ip.ip);
            }
        }
    }
    this->lock.unlockRead();

    for (auto x : addrs) {
        string ip = x.substr(0, x.find(":"));
        if (!DoCheck(ip)) {
            this->lock.lockWrite();
            DeleteAddr(x);
            this->lock.unlockWrite();
        }
    }
}

void MyHandle::HttpAddServer(Request req, Response *resp) {
    rapidjson::Document doc;
    if(doc.Parse(req.body.c_str()).HasParseError()){
        resp->write(200, "{\"success\":false}");
        return;
    }
}

void MyHandle::HttpDelServer(Request req, Response *resp) {
    rapidjson::Document doc;
    if(doc.Parse(req.body.c_str()).HasParseError()){
        resp->write(200, "{\"success\":false}");
        return;
    }
}

void MyHandle::HttpGetAllServer(Request req, Response *resp) {
    rapidjson::StringBuffer s;
    rapidjson::Writer<rapidjson::StringBuffer> w(s);
    w.StartObject();
    w.Key("data");
    w.StartArray();
    this->lock.lockRead();
    for(auto l :server_list_map){
        for(auto x : *l.second){
            w.StartObject();
            w.Key("server");
            w.String(l.first.c_str());
            w.Key("ip");
            w.String(x.ip.c_str());
            w.Key("proportion");
            w.Int(x.proportion);
            w.EndObject();
        }
    }
    w.EndArray();
    w.EndObject();
    std::string json_data = s.GetString();
    resp->set_header("Content-Type", "application/json");
    resp->write(200, json_data);
}

void MyHandle::HttpLogin(Request req, Response *resp) {
    rapidjson::Document doc;
    if(doc.Parse(req.body.c_str()).HasParseError()){
        resp->write(200, "{\"success\":false}");
        return;
    } else {
        if(doc.HasMember("username") && doc.HasMember("password")){
            std::string pass = doc["username"].GetString();
            std::string pwd = doc["password"].GetString();
            if(pass == http_username && pwd == http_pwd){
                resp->write(200, "{\"success\":true}");
                return;
            } else {
                resp->write(200, "{\"success\":false}");
                return;
            }
        } else {
            resp->write(200, "{\"success\":false}");
        }
    }
}
