#include "syscall.h"
#define MAX_LENGTH 64

int main()
{
	int openFileId;
	int fileSize;
	char c; //Ky tu de in ra
	char fileName[MAX_LENGTH];
	int i; //Loop index
	PrintString("\n---------HIEN THI NOI DUNG FILE---------\n");
	PrintString("Nhap vao ten file can doc: ");
	ReadString(fileName, MAX_LENGTH);//Doc ten file, co the thay bang Open(stdin)
	
	openFileId = Open(fileName, 1); //Mo file de doc
	
	if (openFileId != -1) //Neu mo thanh cong
	{
		fileSize = Seek(-1, openFileId); //Seek den cuoi file de lay duoc do dai noi dung file
		i = 0;
		
		Seek(0, openFileId); //Seek den dau tap tin de Read
		PrintString("Noi dung file:\n");
		for (; i < fileSize; i++) // Cho vong lap chay tu 0 --> fileSize
		{
			Read(&c, 1, openFileId); // Goi ham Read de doc tung ki tu noi dung file
			PrintChar(c); // Goi ham PrintChar de in tung ki tu ra man hinh
		}
		Close(openFileId); // Goi ham Close de dong file
	}
	else {
		PrintString(" -> Mo file khong thanh cong!!\n\n");
	}
	return 0;
}