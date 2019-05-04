#include <iostream>
#include <Windows.h>

BOOL InjectDLL(HANDLE hProcess, char* pathDLL);
void InvalidArguments();

int main(int argc, char* argv[])
{
	char pathProcess[1024];
	char pathDLL[1024];
	char pathWorkingFolder[1024];
	char cmdArguments[1024];

	char flags = 0x00;
	const char FLAG_EXE		= 0b00000001;
	const char FLAG_DLL		= 0b00000010;
	const char FLAG_FOLDER	= 0b00000100;
	const char FLAG_ARGS	= 0b00001000;

	if (argc < 5)
	{
		printf("You must specify at least the -exe and -dll arguments.\n");
		InvalidArguments();
		return 1;
	}

	for (int ii = 1; ii < argc-1; ii++)
	{
		if (!strcmp(argv[ii], "-exe"))
		{
			strcpy(pathProcess, argv[ii + 1]);
			flags |= FLAG_EXE;
		}
		else if (!strcmp(argv[ii], "-dll"))
		{
			strcpy(pathDLL, argv[ii + 1]);
			flags |= FLAG_DLL;
		}
		else if (!strcmp(argv[ii], "-folder"))
		{
			strcpy(pathWorkingFolder, argv[ii + 1]);
			flags |= FLAG_FOLDER;
		}
		else if (!strcmp(argv[ii], "-args"))
		{
			strcpy(cmdArguments, argv[ii + 1]);
			flags |= FLAG_ARGS;
		}
	}

	if ((flags & (FLAG_EXE | FLAG_DLL)) != (FLAG_EXE | FLAG_DLL))
	{
		printf("You must specify at least the -exe and -dll arguments.\n");
		InvalidArguments();
		return 1;
	}

	PROCESS_INFORMATION procInfo;
	STARTUPINFO startInfo;
	BOOL procStatus = FALSE;

	ZeroMemory(&procInfo, sizeof(procInfo));
	ZeroMemory(&startInfo, sizeof(startInfo));

	//creates the process with threads suspended, allows us to inject the dll before any files are loaded
	procStatus = CreateProcess(
		pathProcess,
		(flags & FLAG_ARGS) ? cmdArguments : NULL,
		NULL,
		NULL,
		FALSE,
		CREATE_SUSPENDED,
		NULL,
		(flags & FLAG_FOLDER) ? pathWorkingFolder : NULL,
		&startInfo,
		&procInfo
	);

	if (procStatus)
	{
		InjectDLL(procInfo.hProcess, pathDLL);	//inject dll
		ResumeThread(procInfo.hThread);			//resume application

		//cleanup
		CloseHandle(procInfo.hProcess);
		CloseHandle(procInfo.hThread);
	}
	else
	{
		printf("Process creation failed. Ensure the -exe and -dll path arguments are correct.");
		return 2;
	}

	printf("EXE: %s\n", pathProcess);
	printf("DLL: %s\n", pathDLL);
	printf("Injection successful.\n");
	return 0;
}

void InvalidArguments()
{
	printf("Required Arguments:\n");
	printf("\t-exe\t\"exe_file_path\"\t\t\tSpecifies the executable file to be opened\n");
	printf("\t-dll\t\"dll_file_path\"\t\t\tSpecifies the .DLL file to be injected\n");
	printf("Optional Arguments:\n");
	printf("\t-folder\t\"working_folder_path\"\t\t\tSpecifies the startup/working directory of the application\n");
	printf("\t-args\t\"command_line_arguments\"\t\t\tSpecifies the command line arguments passed to the application\n");
	return;
}

BOOL InjectDLL(HANDLE hProcess, char* pathDLL) 
{
	//allocate memory in the application for the dll path
	LPVOID addrDLLName = VirtualAllocEx(hProcess, NULL, strlen(pathDLL), MEM_RESERVE | MEM_COMMIT, PAGE_EXECUTE_READWRITE);
	//write the dll path to memory
	WriteProcessMemory(hProcess, addrDLLName, pathDLL, strlen(pathDLL), NULL);
	//get the address of LoadLibraryA
	LPVOID addrLoadLib = (LPVOID)GetProcAddress(GetModuleHandle("kernel32.dll"), "LoadLibraryA");
	//create a thread for the dll
	CreateRemoteThread(hProcess, NULL, NULL, (LPTHREAD_START_ROUTINE)addrLoadLib, addrDLLName, NULL, NULL);
	return TRUE;
}
