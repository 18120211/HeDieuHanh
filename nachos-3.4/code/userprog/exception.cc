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

// lay tham so tu User space.
int getArg(int i){
	return machine->ReadRegister(i+3);
}

/*
 * int CreateFile(char * name)
 * input:   name: Ten cua file muon tao
 * output:  -1  : Loi
 *           0  : Thanh cong
 */
void CreateFileSyscallHandler(){
	int virtAddr;           	
    char* filename;         //Ten cua file o System space

	DEBUG('a',"\n SC_Create call ...");
	DEBUG('a',"\n Reading virtual address of filename");
	virtAddr = getArg(1);   //Con tro tro den ten file trong User space
	DEBUG ('a',"\n Reading filename.");
	filename = machine->User2System(virtAddr, MAX_FILE_LENGTH + 1);
	if (filename == NULL) //Khong cap phat duoc vung nho trong kernel de luu ten file
	{
        printf("\n Not enough memory in system");
        DEBUG('a',"\n Not enough memory in system");
        machine->WriteRegister(2,-1); // trả về lỗi cho chương trình người dùng
        return;
	}
	DEBUG('a',"\n Finish reading filename.");
	if (!fileSystem->Create(filename,0)){   //Khong tao duoc file moi
        printf("\n Error create file '%s'",filename);
        machine->WriteRegister(2,-1);
        delete[] filename;
        return;
	}
	machine->WriteRegister(2,0); // trả về cho chương trình
	delete[] filename;
}

/*
 * OpenFileID Open(char* name, int type)
 * input:   name: ten cua file muon mo
 *          type: loai file
 * output:  -1  : loi
 *           0  : thanh cong
 */
void OpenFileSyscallHandler(){
    int virtAddr = getArg(1);   //Con tro tro den file muon mo
    int type = getArg(2);       //Loai file muon mo
    if(type < 0 || type > 1){   //Chi mo file "chi doc" hoac "doc va ghi"
        machine->WriteRegister(2, -1);
        return;
    }
    if (fileSystem->size == MAX_OPEN_FILE) { //vuot qua so luong file quan ly
        machine->WriteRegister(2, -1);
        return;
    }

    char * filename = machine->User2System(virtAddr, MAX_FILE_LENGTH);  //Doc ten file vao kernel
    if(type == 1){  //Neu mo file de ghi thi tao truoc khi mo
        fileSystem->Create(filename, 0);
    }

    int fileID = fileSystem->Open(filename, type);  
    if (fileID != -1){  
        DEBUG('f',"open file successfully");
        machine->WriteRegister(2, fileID);  //Neu tao duoc file thi tra ve id cua file
    } 
    else {
        DEBUG('f',"can not open file");
        machine->WriteRegister(2, -1);      //Tra ve -1 neu loi
    }
    delete[] filename;
}

/* 
 * int Close(OpenFileID id)
 * input:   id: id cua file muon dong
 * output:  -1: loi
 *           0: thanh cong
 */
void CloseFileSyscallHandler(){
    int fileID = getArg(1);     //id cua file muon dong
    if (fileID < 2 || fileID >= MAX_OPEN_FILE || fileSystem->openFiles[fileID] == NULL) {
        machine->WriteRegister(2, -1);  //Neu id file khong ton tai thi tra ve -1
        return;
    }
    delete fileSystem->openFiles[fileID];   //Xoa file trong danh sach file quan ly
    fileSystem->openFiles[fileID] = NULL;   
    fileSystem->size--;
    return;
}

/* 
 * int Read(char *buffer, int charcount, OpenFileID id)
 * input:   buffer: vung nho luu chuoi duoc doc
 *          charcount: so ky tu toi da duoc doc
 *          id: id cua file muon doc
 * output:  -1: Loi
 *          -2: File rong
 *          len: so byte thuc su doc duoc
 */
void ReadFileSyscallHandler(){
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
    gSynchConsole->Write(buffer + cur, 1);
    delete[] buffer;
}

//void PrintChar(char c);
void PrintCharSyscallHandler(){
    char c = (char)machine->ReadRegister(4); // Doc ki tu tu thanh ghi r4
	gSynchConsole->Write(&c, 1); // In ky tu tu bien c ra console
}

/* 
 * int Write(char * buffer, int CharCounter, OpenFileId id)
 * input:   buffer: chuoi muon ghi
 *          charCounter: so ki tu toi da ghi duoc
 *          id: id cua file muon ghi
 * output:  -1: Loi
 *          -2: file rong
 *          len: so byte thuc su duoc ghi
 */
void WriteFileSyscallHandler(){
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

/*
 * Seek(int pos, OpenFileId id)
 * input:   pos: vi tri offset muon den
 *          id : id cua file
 * output:  -1 : loi
 *          pos: neu thanh cong
 */
void SeekFileSyscallHandler(){
    int pos = getArg(1);                            //vi tri offset muon den
    int fileID = getArg(2);                         //ID cua file

    if (fileID < 2 || fileID > MAX_OPEN_FILE) {     //Neu file dang khong mo thi tra ve -1
        machine->WriteRegister(2, -1);
        return;
    }

    if (fileSystem->openFiles[fileID] == NULL) {    //ID cua file khong hop le
        machine->WriteRegister(2, -1);
        return;
    }

    if(pos == -1)                                   //neu pos = -1 thi seek den cuoi file
        pos = fileSystem->openFiles[fileID]->Length();

    if (pos > fileSystem->openFiles[fileID]->Length() || pos < 0) {
        machine->WriteRegister(2, -1);              //Offset moi nam ngoai kich thuoc file
    } 
    else {
        fileSystem->openFiles[fileID]->Seek(pos);   //Di chuyen den offset moi
        machine->WriteRegister(2, pos);
    }
}

void StartProcess(int arg){
    currentThread->space->InitRegisters();      //Set the initial register values
    currentThread->space->RestoreState();       //Load page table register
    printf("Run child process\n");
    machine->Run();
    ASSERT(FALSE);
}

//SpaceID Exec(char * filename)
void ExecSyscallHandler(){
    int virtAddr = getArg(1);
    char * filename = machine->User2System(virtAddr, MAX_FILE_LENGTH);
    int fileID = fileSystem->Open(filename);
    OpenFile* executable;
    AddrSpace * space;

    if(fileID == -1)
        executable = NULL;
    else
        executable = fileSystem->openFiles[fileID];

    if(executable == NULL){
        printf("Unable to open file %s\n", filename);
        machine->WriteRegister(2, -1);
        return;
    }
    Thread * newThread = new Thread("new thread");
    machine->WriteRegister(2, newThread->spaceID);
    space = new AddrSpace(executable);
    newThread->space = space;
    newThread->Fork(StartProcess, 0);
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
		case SC_CreateFile:
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
        case SC_Exec:
            ExecSyscallHandler();
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

