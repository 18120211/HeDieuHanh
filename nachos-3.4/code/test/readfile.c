#include "syscall.h"
#include "copyright.h"
#define MAX_LENGTH 255 

//   int strlen(char * s){
//       int i = 0;
//       while(s[i] != '\0' && s[i] != '\n')
//           i++;
//       return i;
//   }

int main(){
    int input;
    int output;
    char filename1[MAX_LENGTH];
    char filename2[MAX_LENGTH];
    char s[MAX_LENGTH];

    PrintString("Nhap ten file input: ");
    Read(filename1, MAX_LENGTH, stdin);

    PrintString("Nhap ten file output: ");
    Read(filename2, MAX_LENGTH, stdin);

    input = Open(filename1, 0);
    output = Open(filename2, 1);

    Seek(1, input);

    if(Read(s, 100, input) > 0){
        Write(s, 100, output);
    } 
    else{
        PrintString("Doc file khong thanh cong");
    }

    Close(output);
    Close(input);
    Halt();
}
