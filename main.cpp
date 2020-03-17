
#include "Server.h"
#include "MyHandle.h"
#include <thread>
#include <iostream>
using namespace std;
#define SERVER_PORT 8527


int main() {
    auto *server = new Server(SERVER_PORT);
    auto *myHandle =  new MyHandle();
    myHandle->HeartCheck();
    server->Init();
    server->Start(myHandle);
    return 0;
}