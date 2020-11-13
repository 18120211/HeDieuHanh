#include "syscall.h"
#define MAX_LENGTH 64

int main()
{
	int srcFileId;  //OpenFileID file nguon
	int destFileId; //OpenFileID file goc
	int fileSize;   //Do dai file
	int i = 0;      //Loop index
	char c;
	char source[MAX_LENGTH];
	char dest[MAX_LENGTH];
	
	PrintString("\n---------SAO CHEP FILE---------\n");
	PrintString("Nhap ten file nguon: ");
    Read(source, MAX_LENGTH, stdin);        //Doc ten file nguon

	PrintString("Nhap ten file dich: ");
    Read(dest, MAX_LENGTH, stdin);          //Doc ten file dich
	srcFileId = Open(source, 0);            //Mo file nguon
	
	if (srcFileId != -1)                    //Neu mo file nguon thanh cong thi
	{
		
		destFileId = Open(dest, 1); //Goi ham Open de mo file dich
		if (destFileId != -1) //Neu mo file dich thanh cong thi
		{
			fileSize = Seek(-1, srcFileId); // Seek den cuoi file nguon de lay duoc do dai noi dung file nguon (fileSize)
			
			Seek(0, srcFileId); //Seek den dau file nguon
			Seek(0, destFileId); //Seek den dau file dich
			
			for (; i < fileSize; i++) //Duyet het file nguon
			{
				Read(&c, 1, srcFileId); //Doc tung ki tu cua file nguon
				Write(&c, 1, destFileId); //Ghi vao file dich
			}
			
			PrintString("Copy thanh cong!\n");
			Close(destFileId); //Dong file dich
		}
		else
		{
			PrintString("Tao file dich khong thanh cong!\n");
		}
		
		Close(srcFileId); //Dong file nguon
	}
	else
	{
		PrintString("Khong mo duoc file.");
	}
	
    Halt();
	return 0;
}
