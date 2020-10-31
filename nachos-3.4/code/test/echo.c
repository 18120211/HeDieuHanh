#include "syscall.h"
#include "copyright.h"
#define MAX_LENGTH 255


int main()
{
    char buffer[MAX_LENGTH];
    int length; 
    PrintString("\n---------ECHO TRONG NACHOS---------\n\n");
    
    length = Read(buffer, MAX_LENGTH, stdin);   //Doc noi dung nhap vao stdin, length la do dai cua chuoi doc vao (neu thanh cong)
    
    if (length != -1 && length != -2)           //Kiem tra co bi loi hoac EOF
    {
        Write(buffer, length, stdout);          //Ghi noi dung doc duoc vao stdout
    }
    
    PrintString("\n\n");
    Halt();
    return 0;
}