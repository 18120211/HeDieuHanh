#include "syscall.h"
#define MAX_LENGTH 64
#define MAX_FILE_SIZE 1000

int main()
{
	int file;
	int fileSize;
    char buffer[MAX_FILE_SIZE];
	char filename[MAX_LENGTH];
    char c;
	int i = 0; //Loop index
	PrintString("Nhap vao ten file can doc: ");
    Read(filename, MAX_LENGTH, stdin);
	
	file = Open(filename, 0);   //Mo file de doc
	
	if (file != -1)       //Neu mo thanh cong
	{
		fileSize = Seek(-1, file); //Seek den cuoi file de lay duoc do dai noi dung file
		Seek(0, file); //Seek den dau tap tin de Read
        for(;i < fileSize;i++){
            Read(&c, 1, file);
            Write(&c, 1, stdout);
        }
		Close(file); // Goi ham Close de dong file
	}
	else {
		PrintString(" -> Mo file khong thanh cong!!\n\n");
	}
	return 0;
}
