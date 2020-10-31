#include "syscall.h"
#include "copyright.h"

int strlen(char * s){
    int i = 0;
    while(s[i] != '\0' && s[i] != '\n')
        i++;
    return i;
}

int main(){
    int input = Open("son.txt", 0);
    int output = Open("ngoc.txt", 1);

    char mes[100];
    while(Read(mes, 100, input) > 0){
        Write(mes, 100, output);
    }

    Close(output);
    Close(input);
    Halt();
}
