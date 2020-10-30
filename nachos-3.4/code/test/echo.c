#include "syscall.h"
#include "copyright.h"
#define MAX_LENGTH 255
int main()
{

	int stdin; //OpenFileID
	int stdout; //OpenFileID
	char buffer[MAX_LENGTH];
	int length; 
	PrintString("\n---------ECHO TRONG NACHOS---------\n\n");
	PrintString("stdin: ");
	
	stdin = Open("stdin", 2);//Mo file stdin, tra ve OpenFileID (int)
	if (stdin != -1)
	{
		length = Read(buffer, MAX_LENGTH, stdin); //Doc noi dung nhap vao stdin, length la do dai cua chuoi doc vao (neu thanh cong)
		
		if (length != -1 && length != -2) //Kiem tra co bi loi hoac EOF
		{
			stdout = Open("stdout", 3); //Mo stdout voi type 3 de ghi
			if (stdout != -1)
			{
				PrintString(" -> stdout: ");
				Write(buffer, length, stdout);//Ghi noi dung doc duoc vao stdout
				Close(stdout); //Dong stdout
			}
		}
		Close(stdin); //Dong stdin
	}
	PrintString("\n\n");
	return 0;

/*
    //Neu cai tren khong chay duoc thi thu ban nay
	char str[255];
	int length = 255;
	PrintString("\n---------ECHO TRONG NACHOS---------\n\n");
	PrintString("Nhap chuoi: ");
	ReadString(str, length);
	PrintString("Chuoi vua nhap: ");
	PrintString(str);
	PrintString("\n\n");
*/
}