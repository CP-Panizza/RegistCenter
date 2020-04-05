
#include "Server.h"
#include "MyHandle.h"
#include "RWLock.hpp"
#include <iostream>
#include <fstream>
#include <map>
#include "libs/http/util.h"
using namespace std;
#define SERVER_PORT 8527


int main() {
    auto conf = getConf("config.txt");
    string user_name;
    string pass_word;
    int heart_check_time;
    if(conf.count("username")){
        user_name = conf["username"];
    }
    if(conf.count("password")){
        pass_word = conf["password"];
    }
    if(conf.count("heart_check_time")){
        heart_check_time = atoi(conf["heart_check_time"].c_str());
    } else {
        heart_check_time = 60;
    }
    auto *server = new Server(SERVER_PORT);
    auto *myHandle =  new MyHandle(user_name, pass_word);
    myHandle->HeartCheck(heart_check_time);
    server->Start(myHandle);

    return 0;
}