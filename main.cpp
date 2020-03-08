
#include "Server.h"
#include "MyHandle.h"
#include <thread>
#include <iostream>
using namespace std;
#define PORT 8527


int main() {
    auto *server = new Server(PORT);
    auto *myHandle =  new MyHandle();
    server->Init();
    server->Start(myHandle);
    return 0;
}