#include "syscall.h"

void main(){
    int i;
    for(i = 0;i<1000;i++)
        PrintString("A");
    Exit(0);
}
