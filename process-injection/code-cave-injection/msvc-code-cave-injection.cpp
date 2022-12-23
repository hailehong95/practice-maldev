#define _CRT_SECURE_NO_WARNINGS
#include <iostream>
#include <Windows.h>
#include <TlHelp32.h>
using namespace std;

/*
** Code Cave - Inject Function to a process
** - https://youtu.be/0NwlWaT9NEY
*/

typedef int(__stdcall *__MessageBoxA)(HWND, LPCSTR, LPCSTR, UINT);

class CaveData
{
public:
	char Message[512];
	char Title[256];
	DWORD FunctionAddr;
};

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

DWORD __stdcall RemoteThread(CaveData *caveData)
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
	cout << "Example: crackme, basecalc,..." << endl;
	cout << "Enter Process Name: "; rewind(stdin);
	cin.getline(strTmp, sizeof(strTmp));
	strcpy(ProcessName, strcat(strTmp, ".exe"));

	cout << "[ STARTING ] Inject Function To Another Process!" << endl;

	CaveData caveData;
	ZeroMemory(&caveData, sizeof(CaveData));

	// copy data
	strcpy(caveData.Message, "Hello Friend!! (o_O)");
	strcpy(caveData.Title, "Process Injection Techniques");

	// get address of MessageBoxA in "user32.dll"
	HINSTANCE hUserModule = LoadLibrary("user32.dll");
	if (!hUserModule)
	{
		cout << "[ ERROR ] Load Library" << endl;
		return 0;
	}

	caveData.FunctionAddr = (DWORD)GetProcAddress(hUserModule, "MessageBoxA");
	FreeLibrary(hUserModule);

	// open target process
	HANDLE hProcess = OpenProcess(PROCESS_ALL_ACCESS, FALSE, GetProcessID(ProcessName));
	if (!hProcess)
	{
		cout << "[ ERROR ] Open Process" << endl;
		return 0;
	}

	// allocation
	LPVOID pRemoteThread = VirtualAllocEx(hProcess, NULL, sizeof(CaveData), MEM_COMMIT | MEM_RESERVE, PAGE_EXECUTE_READWRITE);

	// write
	WriteProcessMemory(hProcess, pRemoteThread, (LPVOID)RemoteThread, sizeof(CaveData), 0);

	// cave code allocation
	CaveData *pData = (CaveData*)VirtualAllocEx(hProcess, NULL, sizeof(CaveData), MEM_COMMIT, PAGE_READWRITE);

	// write data
	WriteProcessMemory(hProcess, pData, &caveData, sizeof(CaveData), NULL);

	// create remote thread
	HANDLE hThread = CreateRemoteThread(hProcess, 0, 0, (LPTHREAD_START_ROUTINE)pRemoteThread, pData, 0, 0);

	// close thread handle
	CloseHandle(hThread);

	// free the now unused memory
	VirtualFreeEx(hProcess, pRemoteThread, sizeof(CaveData), MEM_RELEASE);
	CloseHandle(hProcess);

	cout << "[ OK ] Code Injection Successful...!" << endl;
	cout << "[ EXIT ] Press any key to exit." << endl;

	getchar();
	return 0;
}