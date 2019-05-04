#pragma once

#include <iostream>
#include <Windows.h>
#include <Psapi.h>
#include <TlHelp32.h>
#include <direct.h>

#define GET_BYTE(x,y) ((x >> y*8) & 0xFF)

#define RET_FAIL 1
#define RET_SUCCESS 0

namespace PEFunctions
{
	MODULEINFO GetModuleInfo(char* szModule);

	BOOL FileExists(LPCTSTR szPath);

	void MsgBoxAddr(DWORD64 addr);

	void MsgBoxText(const char* message);

	uint8_t InjectASM(BYTE* addrStart, uint32_t bytesReplaced, BYTE* asmIntructions, size_t asmLength);

	DWORD64 FindPattern(char* modul, char* pattern, char* mask);
	
	template <typename T>
	void FillASM(BYTE* asmDest, T asmSource, uint32_t destStartIndex)
	{
		for (uint32_t ii = 0; ii < sizeof(T); ii++)
			asmDest[destStartIndex + ii] = GET_BYTE(asmSource, ii);

		return;
	}
}