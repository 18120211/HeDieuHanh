#include "syscall.h"

void wait(){
    int i;
    i = 0;
    while(i >= 0){
        i++;
    }
}

int main(){
    int pingPID, pongPID;
    PrintString("Ping-Pong test starting ...");
    pingPID = Exec("./test/ping");
    pongPID = Exec("./test/pong");
    wait();
}
