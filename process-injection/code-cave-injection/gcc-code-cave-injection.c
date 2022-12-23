#include <stdio.h>
#include <windows.h>
#include <tlhelp32.h>

/*
** Code Cave - Inject Function to a process
** - https://youtu.be/0NwlWaT9NEY
*/

typedef int(__stdcall *__MessageBoxA)(HWND, LPCSTR, LPCSTR, UINT);

struct CaveData
{
	char Message[512];
	char Title[256];
	DWORD FunctionAddr;
};
typedef struct CaveData CAVEDATA;

/*
class CaveData
{
public:
	char Message[512];
	char Title[256];
	DWORD FunctionAddr;
};
*/

// Get process id
DWORD GetProcessID(const char* procname)
{
	PROCESSENTRY32 pe;
	HANDLE hSnap;

	pe.dwSize = sizeof(PROCESSENTRY32);
	hSnap = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, NULL);
	if (Process32First(hSnap, &pe)) {
		do
		{
			if (strcmp(pe.szExeFile, procname) == 0)
				break;
		} while (Process32Next(hSnap, &pe));
	}
	return pe.th32ProcessID;
}

DWORD __stdcall RemoteThread(CAVEDATA *caveData)
{
	__MessageBoxA MsgBox = (__MessageBoxA)caveData->FunctionAddr;
	MsgBox(NULL, caveData->Message, caveData->Title, MB_ICONWARNING);
	return EXIT_SUCCESS;
}

int main(int argc, char const *argv[])
{
	system("cls");
	char strTmp[256], ProcessName[256];

	// input process running
	printf("Example: crackme, basecalc,...\n");
	printf("Enter Process Name: ");
	rewind(stdin);
    fgets(strTmp, sizeof(strTmp), stdin);
	strcpy(ProcessName, strcat(strTmp, ".exe"));

    printf("[ STARTING ] Inject Function To Another Process!\n");

	CAVEDATA caveData;
	ZeroMemory(&caveData, sizeof(CAVEDATA));

	// copy data
	strcpy(caveData.Message, "Hello Friend!! (o_O)");
	strcpy(caveData.Title, "Process Injection Techniques");

	// get address of MessageBoxA in "user32.dll"
	HINSTANCE hUserModule = LoadLibrary("user32.dll");
	if (!hUserModule)
	{
	    printf("[ ERROR ] Load Library\n");
		return 0;
	}

	caveData.FunctionAddr = (DWORD)GetProcAddress(hUserModule, "MessageBoxA");
	FreeLibrary(hUserModule);

	// open target process
	HANDLE hProcess = OpenProcess(PROCESS_ALL_ACCESS, FALSE, GetProcessID(ProcessName));
	if (!hProcess)
	{
	    printf("[ ERROR ] Open Process\n");
		return 0;
	}

	// allocation
	LPVOID pRemoteThread = VirtualAllocEx(hProcess, NULL, sizeof(CAVEDATA), MEM_COMMIT | MEM_RESERVE, PAGE_EXECUTE_READWRITE);

	// write
	WriteProcessMemory(hProcess, pRemoteThread, (LPVOID)RemoteThread, sizeof(CAVEDATA), 0);

	// cave code allocation
	CAVEDATA *pData = (CAVEDATA*)VirtualAllocEx(hProcess, NULL, sizeof(CAVEDATA), MEM_COMMIT, PAGE_READWRITE);

	// write data
	WriteProcessMemory(hProcess, pData, &caveData, sizeof(CAVEDATA), NULL);

	// create remote thread
	HANDLE hThread = CreateRemoteThread(hProcess, 0, 0, (LPTHREAD_START_ROUTINE)pRemoteThread, pData, 0, 0);

	// close thread handle
	CloseHandle(hThread);

	// free the now unused memory
	VirtualFreeEx(hProcess, pRemoteThread, sizeof(CAVEDATA), MEM_RELEASE);
	CloseHandle(hProcess);

	printf("[ OK ] Code Injection Successful...!\n");
	printf("[ EXIT ] Press any key to exit.\n");

	getchar();
	return 0;
}
