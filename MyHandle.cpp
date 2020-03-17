//
// Created by Administrator on 2020/3/6.
//


#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include<errno.h>
#include<sys/types.h>
#include<sys/socket.h>
#include<netinet/in.h>
#include<arpa/inet.h>
#include<unistd.h>
#include "MyHandle.h"
#include "libs/rapidjson/document.h"
#include "libs/rapidjson/writer.h"
#include "libs/rapidjson/stringbuffer.h"

using namespace rapidjson;
using namespace std;

MyHandle::MyHandle() {

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
void MyHandle::Server(int connfd, std::string remoteIp, int port) {
    char buff[MAXLINE];
    string respmsg;
    int n;
    n = (int) recv(connfd, buff, MAXLINE, 0);
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
        const Value &serverList = d["ServiceList"];
        for (int i = 0; i < serverList.Size(); ++i) {
            string serName(serverList[i].GetString());
            my_mutex.lock();//加锁读
            if (server_list_map.count(serName)) {
                list<string> *temp = server_list_map[serName];
                temp->push_front(remoteIp);
            } else {
                auto *templist = new list<string>;
                templist->push_front(remoteIp);
                server_list_map[serName] = templist;
            }
            my_mutex.unlock();//加锁读
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

        map<string, list<string> *> temp_map;
        auto serviceList = d["ServiceList"].GetArray();

        for (auto it = serviceList.Begin(); it != serviceList.End(); it++) {
            string ser_name(it->GetString());
            my_mutex.lock(); //加锁读
            if (server_list_map.count(ser_name)) {
                temp_map[ser_name] = server_list_map[ser_name];
            } else {
                list<string> tmep_list;
                temp_map[ser_name] = &tmep_list;
            }
            my_mutex.unlock(); //加锁读
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
                writer.String(ip.c_str());
            }
            writer.EndArray();
            writer.EndObject();
        }
        writer.EndArray();
        writer.EndObject();
        respmsg = string(s.GetString());
    }

    if (send(connfd, (void *) respmsg.c_str(), strlen(respmsg.c_str()), 0) < 0) {
        printf("send msg error: %s(errno: %d)\n", strerror(errno), errno);
    }

    close(connfd);
}


/**
 * 轮训检测服务端是否在线
 */
void MyHandle::HeartCheck() {
    cout << "start heartCheck" << endl;
    thread t(&MyHandle::HeartCheckEntry, this);
    t.detach();
}


void MyHandle::HeartCheckEntry() {
    list<string> addrs; //存放去重复后的远端服务器地址
    while (true) {
        if (server_list_map.empty()) {
            this_thread::sleep_for(std::chrono::milliseconds(10000));
            continue;
        }


        for (auto l : server_list_map) {
            for (auto ip : *(l.second)) {
                if (!count(addrs, ip)) {
                    addrs.push_front(ip);
                }
            }
        }

        for (auto x : addrs) {
            auto pos = x.find(":");
            string ip = x.substr(0, pos);
            bool ok = DoCheck(ip);
            if (!ok) {
                my_mutex.lock();
                DeleteAddr(x);
                my_mutex.unlock();
            }
        }
        addrs.clear();
        this_thread::sleep_for(std::chrono::minutes(3));
    }
}


bool count(const list<string> &l, string target) {
    for (auto x : l) {
        if (x == target)
            return true;
    }
    return false;
}


bool MyHandle::DoCheck(const string &ip) {
    int sockfd, n;
    char recvline[100], sendline[] = "CHECK";
    struct sockaddr_in servaddr;
    if ((sockfd = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
        printf("create socket error: %s(errno: %d)\n", strerror(errno), errno);
        return true;
    }
    memset(&servaddr, 0, sizeof(servaddr));
    servaddr.sin_family = AF_INET;
    servaddr.sin_port = htons(CLIENT_PORT);
    if (inet_pton(AF_INET, ip.c_str(), &servaddr.sin_addr) <= 0) {
        printf("inet_pton error for %s\n", ip.c_str());
        return true;
    }

    if (connect(sockfd, (struct sockaddr *) &servaddr, sizeof(servaddr)) < 0) {
        printf("connect error: %s(errno: %d)\n", strerror(errno), errno);
        return false;
    }

    if (send(sockfd, sendline, strlen(sendline), 0) < 0) {
        printf("send msg error: %s(errno: %d)\n", strerror(errno), errno);
        return false;
    }

    if ((n = (int) recv(sockfd, recvline, sizeof(recvline), 0)) < 0) {
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
            x.second->remove(ip);
            if (x.second->empty()) {
                emptyListName.push_back(x.first);
            }
        }
    }

    for (auto x : emptyListName) {
        server_list_map.erase(x);
    }
}
