#include <iostream>
#include <Windows.h>
#include <Psapi.h>
#include <TlHelp32.h>
#include <direct.h>
#include "PEFunctions.h"

namespace PEFunctions
{
	MODULEINFO GetModuleInfo(char* szModule)
	{
		MODULEINFO modInfo = { 0 };
		HMODULE hModule = GetModuleHandle(szModule);

		if (hModule == NULL)
			return modInfo;

		GetModuleInformation(GetCurrentProcess(), hModule, &modInfo, sizeof(MODULEINFO));
		return modInfo;
	}

	BOOL FileExists(LPCTSTR szPath)
	{
		DWORD dwAttrib = GetFileAttributes(szPath);

		return (dwAttrib != INVALID_FILE_ATTRIBUTES && !(dwAttrib & FILE_ATTRIBUTE_DIRECTORY));
	}

	void MsgBoxAddr(DWORD64 addr)
	{
		char szBuffer[1024];
		sprintf(szBuffer, "Address: %llx", addr);
		MessageBox(NULL, szBuffer, "Debug Message", MB_OK);
	}

	void MsgBoxText(const char* message)
	{
		MessageBox(NULL, message, "Debug Message", MB_OK);
	}

	DWORD64 FindPattern(char* modul, char* pattern, char* mask)
	{
		MODULEINFO mInfo = GetModuleInfo(modul);

		DWORD64 base = (DWORD64)mInfo.lpBaseOfDll;
		DWORD64 size = (DWORD64)mInfo.SizeOfImage;

		DWORD patternLength = (DWORD)strlen(mask);

		for (DWORD64 ii = 0; ii < size - patternLength; ii++)
		{
			bool found = true;
			for (DWORD64 jj = 0; jj < patternLength; jj++)
			{
				found &= ((mask[jj] == '?') || (pattern[jj] == *(char*)(base + ii + jj)));
			}
				
			if (found)
			{
				return base + ii;
			}

		}

		return NULL;
	}

	uint8_t InjectASM(BYTE* addrStart, uint32_t addrLength, BYTE* asmIntructions, size_t asmLength)
	{
		//addrStart			-- address to start replacing bytes from (includes this address)
		//addrLength		-- number of bytes in program memory to replace
		//asmInstructions	-- array of opcode bytes
		//asmLength			-- number of bytes in asmInstructions, must be less than addrLength

		//check if the instructions will fit into the addresses
		if (asmLength > addrLength)
			return RET_FAIL;

		//give us R/W/E permissions
		DWORD dwOldProtect, dwBackup;
		VirtualProtect(addrStart, addrLength, PAGE_EXECUTE_READWRITE, &dwOldProtect);		

		//inject the assembly instructions
		for (uint32_t ii = 0; ii < asmLength; ii++)
			*(addrStart + ii) = asmIntructions[ii];

		//replace remaining bytes with NOPs
		for (uint32_t ii = asmLength; ii < addrLength; ii++)
			*(addrStart + ii) = 0x90;
			
		//restore old permissions
		VirtualProtect(addrStart, addrLength, dwOldProtect, &dwBackup);

		return RET_SUCCESS;
	}
}
