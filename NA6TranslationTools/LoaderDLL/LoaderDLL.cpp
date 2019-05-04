#include "..\PEFunctions.h"
#include "LoaderDLL.h"

#define INDEX_PLACEHOLDER 3
#define OFFSET_JUMPBACK 13
#define BYTES_TO_REPLACE 22

#define dim(x) (sizeof(x) / sizeof((x)[0]))

using namespace PEFunctions;

void InitiateHooks();
__declspec(naked) void InitCodeCave(void);
void ExecCodeCave(void);

BYTE* fileContents;
char* filePath;
DWORD fileSize;
DWORD fileLooseSize;

DWORD_PTR addrJumpBack;
DWORD64 regRAX;

BYTE asmLoader[] = {
	0x50,		//push rax
	0x48, 0xB8, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,	//mov rax, ADDR_PLACEHOLDER
	0xFF, 0xE0,	//jmp rax
	0x58,		//pop rax
	0x9D		//popfq
};

void InitiateHooks()
{
	char procName[] = "xrEngine.exe";
	char patBytes[] = "\x8B\x4D\x14\x48\x8B\xF8\x89\x48\x14\x49\x8B\xCC\x48\x89\x58\x08\x89\x70\x10\x89\x70\x18";
	char patMask[] = "xxxxxxxxxxxxxxxxxxxxxx";

	DWORD64 addrEntry = FindPattern(procName, patBytes, patMask);
	if (addrEntry != NULL)
	{
		//fill in the placeholder for the jumpTo address
		//	this makes the program jump into our dll init code
		FillASM(asmLoader, (DWORD64)InitCodeCave, INDEX_PLACEHOLDER);

		//set the jumpBack address
		//	the jumpBack address points to the pop rax instruction
		addrJumpBack = addrEntry + OFFSET_JUMPBACK;
		InjectASM((BYTE*)addrEntry, BYTES_TO_REPLACE, asmLoader, dim(asmLoader));
	}
	else
	{
		MsgBoxText("No suitable injection point found.\nxrEngine.exe may have been updated.");
	}

	return;
}

__declspec(naked) void InitCodeCave(void)
{
	__asm
	{
		//additional asm
		pop rax;				//restore rax

		//original asm
		mov ecx, dword ptr ss : [rbp + 0x14];

		//additional asm
		mov fileSize, ecx;		//copy the size of the file
		mov regRAX, rax;		//copy rax, possibly needed later

		//original asm
		mov rdi, rax;
		mov dword ptr ds : [rax + 0x14], ecx;
		mov rcx, r12;
		mov qword ptr ds : [rax + 0x08], rbx;
		mov dword ptr ds : [rax + 0x10], esi;
		mov dword ptr ds : [rax + 0x18], esi;

		//additional asm
		mov fileContents, rbx;	//copy file_contents byte*
		lea rbx, [rsp + 0x80];
		mov filePath, rbx;		//copy file_path char*
		mov rbx, fileContents;

		//preserve system state for C code exec
		pushfq;
		push rax;
		push rbx;
		push rcx;
		push rdx;
		push rsi;
		push rdi;
		push rbp;
		push r8;
		push r9;
		push r10;
		push r11;
		push r12;
		push r13;
		push r14;
		push r15;
	}

	ExecCodeCave();

	__asm
	{
		//restore system state
		pop r15;
		pop r14;
		pop r13;
		pop r12;
		pop r11;
		pop r10;
		pop r9;
		pop r8;
		pop rbp;
		pop rdi;
		pop rsi;
		pop rdx;
		pop rcx;
		pop rbx;

		//jump back to original code
		mov rax, addrJumpBack;
		jmp rax;
	}
}

void ExecCodeCave(void)
{
	if (filePath != NULL)
	{
		if (FileExists(filePath))
		{
			HANDLE hFile = CreateFile(filePath, GENERIC_READ, 0, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
			if (hFile != INVALID_HANDLE_VALUE)
			{
				fileLooseSize = GetFileSize(hFile, NULL);	//get size of the loose file in bytes
				BYTE* fileLooseBuffer = (BYTE*)HeapAlloc(GetProcessHeap(), NULL, fileLooseSize + 16);	//allocate new buffer (the disasm malloc has the +16, not sure why)

				HeapFree(GetProcessHeap(), NULL, fileContents);	//free the old buffer
				DWORD szBuffer;
				ReadFile(hFile, fileLooseBuffer, fileLooseSize, &szBuffer, NULL);	//load the file contents into our new buffer
				_asm
				{
					push rcx;
					push rax;

					mov ecx, fileLooseSize;
					mov rax, regRAX;
					mov dword ptr ds : [rax + 0x14], ecx;	//overwrite stored buffer size to our new buffer size
					mov rcx, fileLooseBuffer;
					mov qword ptr ds : [rax + 0x08], rcx;	//redirect buffer pointer file_contents to our malloc

					pop rax;
					pop rcx;
				}
			}
			CloseHandle(hFile);
		}
	}

	return;
}

