#include "syscall.h"
#include "copyright.h"
#define MAX_LENGTH 32

int main()
{
		int len;
		char filename[MAX_LENGTH+1];
        PrintString("Nhap ten file muon tao: ");
        Read(filename, MAX_LENGTH, stdin);
        if(CreateFile(filename) == 0){
            PrintString("Tao file thanh cong");
        }
        else{
            PrintString("Tao file khong thanh cong");
        }
		Halt();
}
