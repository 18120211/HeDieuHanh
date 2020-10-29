#include "syscall.h"
#include "copyright.h"
#define maxlen 32

int main()
{
		int len;
		char filename[maxlen+1];
        if(Create("son.txt") == -1){
            PrintString("Tao file khong thanh cong");
        }
        else{
            PrintString("Tao file thanh cong");
        }
		Halt();
}
