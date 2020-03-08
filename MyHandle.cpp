//
// Created by Administrator on 2020/3/6.
//



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
            if (server_list_map.count(serName)) {
                list<string> *temp = server_list_map[serName];
                my_mutex.lock();
                temp->push_front(remoteIp);
                my_mutex.unlock();
            } else {
                list<string> *templist = new list<string>;
                templist->push_front(remoteIp);
                my_mutex.lock();
                server_list_map[serName] = templist;
                my_mutex.unlock();
            }
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
            if (server_list_map.count(ser_name)) {
                temp_map[ser_name] = server_list_map[ser_name];
            } else {
                list<string> tmep_list;
                temp_map[ser_name] = &tmep_list;
            }
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
            for (auto ip : *key_val.second) {
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