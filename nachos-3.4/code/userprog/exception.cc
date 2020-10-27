// exception.cc 
//	Entry point into the Nachos kernel from user programs.
//	There are two kinds of things that can cause control to
//	transfer back to here from user code:
//
//	syscall -- The user code explicitly requests to call a procedure
//	in the Nachos kernel.  Right now, the only function we support is
//	"Halt".
//
//	exceptions -- The user code does something that the CPU can't handle.
//	For instance, accessing memory that doesn't exist, arithmetic errors,
//	etc.  
//
//	Interrupts (which can also cause control to transfer from user
//	code into the Nachos kernel) are handled elsewhere.
//
// For now, this only handles the Halt() system call.
// Everything else core dumps.
//
// Copyright (c) 1992-1993 The Regents of the University of California.
// All rights reserved.  See copyright.h for copyright notice and limitation 
// of liability and disclaimer of warranty provisions.

#include "copyright.h"
#include "system.h"
#include "syscall.h"
#define MaxFileLength 32

//----------------------------------------------------------------------
// ExceptionHandler
// 	Entry point into the Nachos kernel.  Called when a user program
//	is executing, and either does a syscall, or generates an addressing
//	or arithmetic exception.
//
// 	For system calls, the following is the calling convention:
//
// 	system call code -- r2
//		arg1 -- r4
//		arg2 -- r5
//		arg3 -- r6
//		arg4 -- r7
//
//	The result of the system call, if any, must be put back into r2. 
//
// And don't forget to increment the pc before returning. (Or else you'll
// loop making the same system call forever!
//
//	"which" is the kind of exception.  The list of possible exceptions 
//	are in machine.h.
//----------------------------------------------------------------------

char* User2System(int Address, int limit)
{
	int i; //index
	int oneChar;
	char* kernelBuffer = NULL;
	kernelBuffer = new char[limit + 1]; //need for terminal string
	if(kernelBuffer == NULL)
		return kernelBuffer;
	memset(kernelBuffer, 0, limit + 1);
	//printf("\n Filename u2s:");
	for(i = 0;i < limit;i++)
	{
		machine->ReadMem(Address+i,1,&oneChar);
		kernelBuffer[i] = (char)oneChar;
		//printf("%c", kernelBuffer[i]);
		if(oneChar == 0)
			break;
	}
	return kernelBuffer;
}

// Input: - User space address (int)
// - Limit of Bufferfer (int)
// - Bufferfer (char[])
// Output:- Number of bytes copied (int)
// Purpose: Copy Bufferfer from System memory space to User memory space
int System2User(int Address,int len,char* Bufferfer)
{
		if (len < 0) return -1;
		if (len == 0)return len;
		int i = 0;
		int oneChar = 0 ;
		do{
				oneChar= (int) Bufferfer[i];
				machine->WriteMem(Address+i,1,oneChar);
				i++;
		}while(i < len && oneChar != 0);
		return i;
}

//Increment Program Counter Register
void IncPCReg()
{
	int PC_Register = machine->ReadRegister(PCReg);
	int Next_PC_Register = machine->ReadRegister(NextPCReg);
	machine->WriteRegister(PrevPCReg, PC_Register);
	machine->WriteRegister(PCReg, Next_PC_Register);
	machine->WriteRegister(NextPCReg, Next_PC_Register + 4);
}

void CreateFileSysCallHandler()
{
	int Address;
	char* filename;

	DEBUG('a',"\n SC_Create call ...");
	DEBUG('a',"\n Reading virtual address of filename");
	// Lấy tham số tên tập tin từ thanh ghi r4
	Address = machine->ReadRegister(4);
	DEBUG ('a',"\n Reading filename.");
	filename = User2System(Address,MaxFileLength+1);
	if (filename == NULL)
	{
			printf("\n Not enough memory in system");
			DEBUG('a',"\n Not enough memory in system");
			machine->WriteRegister(2,-1); // trả về lỗi cho chương trình người dùng
			delete filename;
			return;
	}
	DEBUG('a',"\n Finish reading filename.");
	if (!fileSystem->Create(filename,0))
	{
			printf("\n Error create file '%s'",filename);
			machine->WriteRegister(2,-1);
			delete filename;
			return;
	}
	machine->WriteRegister(2,0); // trả về cho chương trình
	delete filename;
}

void OpenFileSyscallHandler(){}
void CloseFileSyscallHandler(){}
void ReadFileSyscallHandler(){
	// int Read(char *buffer, int charcount, OpenFileID id)
	// Return -1: FAIL
	// Return So byte doc duoc: Succeed
	// Return -2: Thanh cong
	int PrevAddress;
	int NextAddress;
	char *Buffer;	
	int Address = machine->ReadRegister(4); // Address = gia tri thanh ghi 4
	int CharCounter = machine->ReadRegister(5); // CharCounter = gia tri thanh ghi so 5
	int id = machine->ReadRegister(6); // Lay id cua file tu thanh ghi so 6 
	
	if (fileSystem->openf[identifier] == NULL){// File khong ton tai tra ve -1 
		printf("File khong ton tai");
		machine->WriteRegister(2, -1);
		IncreasePC();
		return;
	}
	if (fileSystem->openf[identifier]->type == 3) {// Xet truong hop doc file stdout (type quy uoc la 3) thi tra ve -1 
		printf("File loai STDOUT nen khong doc duoc");
		machine->WriteRegister(2, -1);
		IncreasePC();
		return;
	}
	if (identifier > 14 || identifier < 0) {// identifier nam ngoai pham vi thi tra ve -1
		printf("File khong phu hop");
		machine->WriteRegister(2, -1);
		IncreasePC();
		return;
	}
	
	PrevAddress = fileSystem->openf[identifier]->GetCurrentPos(); // Luu gia tri file vua mo vao PrevAddress
	Buffer = User2System(Address, CharCounter); // Chuyen du lieu la vung nho Buffer co CharCounter byte tu Usermode->Systemode
	if (fileSystem->openf[identifier]->type == 2) {
		length
		int length = gSynchConsole->Read(Buffer, CharCounter); //length = so byte doc duoc tu thang Buffer
		System2User(Address, length, Buffer); // Chuyen du lieu la vung nho Buffer co length byte tu Systemmode->Usermode
		machine->WriteRegister(2, length); // Thanh ghi so 2 luu gia tri so byte doc duoc
		delete Buffer;
		IncreasePC();
		return;
	}
	if ((fileSystem->openf[identifier]->Read(Buffer, CharCounter)) > 0)  {// Doc file thanh cong voi so ky tu doc duoc > 0
		NextAddress = fileSystem->openf[identifier]->GetCurrentPos(); // So byte doc duoc = NextAddress - PrevAddress
		System2User(Address, NextAddress - PrevAddress, Buffer); // Chuyen du lieu tu la chuoi Buffer co so byte = so byte doc duoc tu Systemspace sang Userspace
		machine->WriteRegister(2, NextAddress - PrevAddress);
	}
	else {
		//Doc duoc file co do dai = 0;
		printf("File rong");
		machine->WriteRegister(2, -2);
	}
	delete Buffer;
	IncreasePC();
	return;
}
void WriteFileSyscallHandler(){}

void SyscallExceptionHandler(int type)
{
	switch (type){
		case SC_Halt:
			DEBUG('a', "\n Shutdown, initiated by user program.");
			printf ("\n\n Shutdown, initiated by user program.");
			interrupt->Halt();
			break;
		case SC_Create:
			CreateFileSysCallHandler();
			break;
		case SC_Open:
			break;
		case SC_Read:
			ReadFileSyscallHandler();
			break;
		case SC_Write:
			break;
		case SC_Close:
			break;
		default:
			printf("\n SyscalException: Unexpected system call %d", type);
			interrupt->Halt();
	}
	IncPCReg();
}

void
ExceptionHandler(ExceptionType which)
{
	switch (which) {
		case NoException:
			return;
		case PageFaultException:
			printf("No valid translation found.\n");
			interrupt->Halt();
			break;
		case ReadOnlyException:
			printf("Write attempted to page marked \"read-only\"\n");
			interrupt->Halt();
			break;
		case BusErrorException:
			printf("Translation resulted in an invalid physical address\n");
			interrupt->Halt();
			break;
		case AddressErrorException:
			printf("Unaligned reference or one that was beyond the end of the address space\n");
			interrupt->Halt();
			break;
		case OverflowException:
			printf("Integer overflow in add or sub.\n");
			interrupt->Halt();
			break;
		case IllegalInstrException:
			printf("Unimplemented or reserved instr.\n");
			interrupt->Halt();
			break;
		case NumExceptionTypes:
			printf("Number exception types\n");
			interrupt->Halt();
			break;
		case SyscallException:
			int type = machine->ReadRegister(2);
			SyscallExceptionHandler(type);
			DEBUG('a', "\n Incrementing PC.");
			IncPCReg();
			break;
		default:
			printf("\n Unexpected user mode exception (%d %d)", which, type);	
	}
}
