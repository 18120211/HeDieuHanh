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
#include "string.h"
#define MAX_FILE_LENGTH 32
#define MAX_STRING_LENGTH 100

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

int getArg(int i){
	return machine->ReadRegister(i+3);
}

void CreateFileSyscallHandler(){
	int virtAddr;
	char* filename;

	DEBUG('a',"\n SC_Create call ...");
	DEBUG('a',"\n Reading virtual address of filename");
	virtAddr = machine->ReadRegister(4);
	DEBUG ('a',"\n Reading filename.");
	filename = machine->User2System(virtAddr, MAX_FILE_LENGTH + 1);
	if (filename == NULL)
	{
        printf("\n Not enough memory in system");
        DEBUG('a',"\n Not enough memory in system");
        machine->WriteRegister(2,-1); // trả về lỗi cho chương trình người dùng
        delete[] filename;
        return;
	}
	DEBUG('a',"\n Finish reading filename.");
	if (!fileSystem->Create(filename,0)){
        printf("\n Error create file '%s'",filename);
        machine->WriteRegister(2,-1);
        delete[] filename;
        return;
	}
	machine->WriteRegister(2,0); // trả về cho chương trình
	delete[] filename;
}

//OpenFileID Open(char* name, int type)
void OpenFileSyscallHandler(){
    int virtAddr = getArg(1);
    int type = getArg(2);
    if(type < 0 || type > 1){
        machine->WriteRegister(2, -1);
        return;
    }
    if (fileSystem->size > 10) {
        machine->WriteRegister(2, -1);
        return;
    }

    char * filename = machine->User2System(virtAddr, MAX_FILE_LENGTH);
    if(type == 1){
        fileSystem->Create(filename, 0);
    }

    int fileID = fileSystem->Open(filename, type);
    if (fileID != -1){
        DEBUG('f',"open file successfully");
        machine->WriteRegister(2, fileID);
    } 
    else {
        DEBUG('f',"can not open file");
        machine->WriteRegister(2, -1);
    }
    delete[] filename;
}

//int Close(OpenFileID id)
void CloseFileSyscallHandler(){
    int fileID = getArg(1);
    if (fileID < 0 || fileID > 1 || fileSystem->openFiles[fileID] == NULL) {
        machine->WriteRegister(2, -1);
        return;
    }
    delete fileSystem->openFiles[fileID];
    fileSystem->openFiles[fileID] = NULL;
    fileSystem->size--;
    return;
}

void ReadFileSyscallHandler(){
	// int Read(char *buffer, int charcount, OpenFileID id)
	// Return -1: FAIL
	// Return So byte doc duoc: Succeed
	// Return -2: Thanh cong
    int PrevAddress;
    int NextAddress;
    int Address = getArg(1);                               //Address = gia tri thanh ghi 4
    int CharCounter = getArg(2);                           //CharCounter = gia tri thanh ghi so 5
    int id = getArg(3);                                    //Lay id cua file tu thanh ghi so 6
    if (fileSystem->openFiles[id] == NULL) {               //File khong ton tai tra ve -1
        machine->WriteRegister(2, -1);
        return;
    }

    if (id < 0 || id > MAX_OPEN_FILE) {
        machine->WriteRegister(2, -1);
        return;
    }

    if(fileSystem->openFiles[id]->type == 3){               //Xet truong hop doc stdout
        machine->WriteRegister(2, -1);
        return;
    }

    char *Buffer = new char[CharCounter + 1];
    PrevAddress = fileSystem->openFiles[id]->GetCurrentPos();   //Luu lai vi tri con tro file
    if (fileSystem->openFiles[id]->type == 2) {                 //Doc qua stdin
        int length = gSynchConsole->Read(Buffer, CharCounter);  //length so byte thuc te 
        machine->System2User(Address, length, Buffer);          //Chuyen du lieu System->User
        machine->WriteRegister(2, length);                      //tra ve so byte doc duoc
        delete[] Buffer;
        return;
    }
    
    if ((fileSystem->openFiles[id]->Read(Buffer, CharCounter)) > 0){   //Doc file thanh cong
        NextAddress = fileSystem->openFiles[id]->GetCurrentPos();       
        machine->System2User(Address, NextAddress - PrevAddress + 1, Buffer);
        machine->WriteRegister(2, NextAddress - PrevAddress + 1);
    }
    else {
        machine->WriteRegister(2, -2);
    }
    delete[] Buffer;
    return;
}

//void ReadString(char * s, int length);
void ReadStringSyscallHandler(){ //Cai nay la Phuong viet
    int virtAddr = getArg(1); //Doc register 4, lay dia chi tham so buffer
    int length = getArg(2); //Doc register 5, lay do dai toi da cua chuoi nhap vao
    char *buffer;
    buffer = machine->User2System(virtAddr,length);//Copy chuoi tu User Space sang System Space
    gSynchConsole->Read(buffer, length);//Goi ham Read cua class SynchConsole de doc chuoi nguoi dung nhap
    machine->System2User(virtAddr,length,buffer);//Copy chuoi tu System Space sang User Space
    delete[] buffer;
}

//void PrintString(char * s);
void PrintStringSyscallHandler(){
    int virtAddr = getArg(1); //Doc register 4
    int cur = 0;
    char *buffer;
    buffer = machine->User2System(virtAddr, MAX_STRING_LENGTH);
    while (buffer[cur] != 0 && buffer[cur] != '\n'){
        gSynchConsole->Write(buffer+cur, 1);
        cur++;
    }
    buffer[cur] = '\n';
    gSynchConsole->Write(buffer + cur, 1);
    delete[] buffer;
}

//void PrintChar(char c);
void PrintCharSyscallHandler(){
    char c = (char)machine->ReadRegister(4); // Doc ki tu tu thanh ghi r4
	gSynchConsole->Write(&c, 1); // In ky tu tu bien c ra console
}

void WriteFileSyscallHandler(){
    // int Write(char * buffer, int CharCounter, OpenFileidentifier identifier)
	// Return -1: FAIL
	// Return So byte ghi duoc: Succeed
	// Return -2: Succeed
    int PrevAddress;
    int NextAddress;
    int Address = machine->ReadRegister(4);
    int CharCounter = machine->ReadRegister(5);
    int id =  machine->ReadRegister(6);

    if (fileSystem->openFiles[id] == NULL) {
        machine->WriteRegister(2, -1);
        return;
    }

    if (id < 0 || id > MAX_OPEN_FILE) {
        machine->WriteRegister(2, -1);
        return;
    }

    if(fileSystem->openFiles[id]->type % 2 == 0){            //Read only or stdin -> can't write on it
        machine->WriteRegister(2, -1);
        return;
    }

    PrevAddress = fileSystem->openFiles[id]->GetCurrentPos();
    char * Buffer = machine->User2System(Address, CharCounter);

    //Write to console(stdout)
    if(fileSystem->openFiles[id]->type == 3){
        int cur = 0;
        while(Buffer[cur] != 0 && Buffer[cur] != '\n'){
            gSynchConsole->Write(Buffer + cur, 1);
            cur++;
        }
        gSynchConsole->Write(Buffer + cur, 1);
        machine->WriteRegister(2, cur - 1);
    }

    //Write to file
    if (fileSystem->openFiles[id]->Write(Buffer, strlen(Buffer)) > 0) {
        NextAddress = fileSystem->openFiles[id]->GetCurrentPos();
        machine->WriteRegister(2, NextAddress - PrevAddress + 1);
    }
    delete[] Buffer;
    return;
}

void SeekFileSyscallHandler(){
    int pos = machine->ReadRegister(4);
    int fileID = machine->ReadRegister(5);

    if (fileID < 0 || fileID > MAX_OPEN_FILE) {
        machine->WriteRegister(2, -1);
        return;
    }

    if (fileSystem->openFiles[fileID] == NULL) {
        machine->WriteRegister(2, -1);
        return;
    }

    if(pos == -1)
        pos = fileSystem->openFiles[fileID]->Length();

    if (pos > fileSystem->openFiles[fileID]->Length() || pos < 0) {
        machine->WriteRegister(2, -1);
    } 
    else {
        fileSystem->openFiles[fileID]->Seek(pos);
        machine->WriteRegister(2, pos);
    }
}

void SyscallExceptionHandler(int type)
{
	switch (type){
		case SC_Halt:
			DEBUG('a', "\n Shutdown, initiated by user program.");
			printf ("\n\n Shutdown, initiated by user program.");
			interrupt->Halt();
			break;
		case SC_ReadString:
			ReadStringSyscallHandler();
			break;
		case SC_PrintString:
			PrintStringSyscallHandler();
			break;
		case SC_Create:
			CreateFileSyscallHandler();
			break;
		case SC_Open:
			OpenFileSyscallHandler();
			break;
		case SC_Read:
			ReadFileSyscallHandler();
			break;
		case SC_Write:
            WriteFileSyscallHandler();
			break;
        case SC_Seek:
            SeekFileSyscallHandler();
            break;
		case SC_Close:
			CloseFileSyscallHandler();
			break;
	}
	machine->registers[PrevPCReg] = machine->registers[PCReg];	
	machine->registers[PCReg] = machine->registers[NextPCReg];
	machine->registers[NextPCReg] += 4;
}

void ExceptionHandler(ExceptionType which)
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
			break;
		default:
			printf("\n Unexpected user mode exception (%d %d)", which, type);	
	}
}

