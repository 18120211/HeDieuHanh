#include "syscall.h"
#include "copyright.h"

int strlen(char * s){
    int i = 0;
    while(s[i] != '\0' && s[i] != '\n')
        i++;
    return i;
}

int main(){
    int input = Open("input.txt", 2);
    int output = Open("output.txt", 1);
    char mes[100];
    if(Read(mes, 100, input) == -1){
        PrintString("Doc file khong thanh cong");
    }
    else{
        Seek(1, output);
        Write(mes, strlen(mes), output);
    }
    Close(input);
    Close(output);
    Halt();
}
