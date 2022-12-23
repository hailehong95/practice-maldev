#define _CRT_SECURE_NO_WARNINGS
#include <iostream>
#include <Windows.h>
#include <TlHelp32.h>
using namespace std;

/*
** Hook Injection Via Setwindowshookex
** Reference:
** [1] - https://www.endgame.com/blog/technical-blog/ten-process-injection-techniques-technical-survey-common-and-trending-process
** [2] - http://www.rohitab.com/discuss/topic/43926-setwindowshookex-dll-injection-my-code-and-some-questions/
** [3] - https://attack.mitre.org/wiki/Technique/T1055
*/

// Get The Process ID
DWORD GetProcessID(const char* procname)
{
	PROCESSENTRY32 pe;
	HANDLE hSnap;

	pe.dwSize = sizeof(PROCESSENTRY32);
	hSnap = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, NULL);
	if (Process32First(hSnap, &pe)) {
		do
		{
			if (strcmp(pe.szExeFile, procname) == 0)
				break;
		} while (Process32Next(hSnap, &pe));
	}
	return pe.th32ProcessID;
}

int main(int argc, char const *argv[])
{
	char ProcessName[512], dllPathTmp[512];
	cout << "Right click Taskbar > Task Manager > View Process running" << endl;
	cout << "Example: Base Calculator, CrackMe v1.0, HxD,..." << endl;
	cout << "Enter Process Name: "; rewind(stdin);
	cin.getline(ProcessName, sizeof(ProcessName));

	// DLL Path
	cout << "Enter DLL Path: ";
	cin.getline(dllPathTmp, sizeof(dllPathTmp));

	cout << "[ Starting ] Hook Injection Via Setwindowshookex" << endl;

	// Finding target window
	HWND hwnd = FindWindow(NULL, ProcessName);
	if (hwnd == NULL) {
		cout << "[ FAILED ] Could not find target window." << endl; return EXIT_FAILURE;
	}

	// Getting the thread of the window and the PID
	DWORD processID = NULL;
	DWORD threadID = GetWindowThreadProcessId(hwnd, &processID);
	if (threadID == NULL) {
		cout << "[ FAILED ] Could not get thread ID of the target window." << endl; return EXIT_FAILURE;
	}

	// Loading DLL
	HMODULE dll = LoadLibraryEx(dllPathTmp, NULL, DONT_RESOLVE_DLL_REFERENCES);
	if (dll == NULL) {
		cout << "[ FAILED ] The DLL could not be found." << endl; return EXIT_FAILURE;
	}

	// Getting exported function address: using GetProcAddress();
	HOOKPROC addr = (HOOKPROC)GetProcAddress(dll, "myFunction");
	if (addr == NULL) {
		cout << "[ FAILED ] The function was not found." << endl; return EXIT_FAILURE; return EXIT_FAILURE;
	}

	// Setting the hook in the hook chain
	// Or WH_KEYBOARD if you prefer to trigger the hook manually
	HHOOK handle = SetWindowsHookEx(WH_GETMESSAGE, addr, dll, threadID);
	if (handle == NULL) {
		cout << "[ FAILED ] Couldn't set the hook with SetWindowsHookEx." << endl; return EXIT_FAILURE;
	}

	// Triggering the hook
	PostThreadMessage(threadID, WM_NULL, NULL, NULL);

	// Waiting for user input to remove the hook
	cout << "[ OK ] Hook set and triggered." << endl;
	cout << "[ >> ] Press any key to unhook (This will unload the DLL)." << endl;
	system("pause > nul");

	// Unhooking
	BOOL unhook = UnhookWindowsHookEx(handle);
	if (unhook == FALSE) {
		cout << "[ FAILED ] Could not remove the hook." << endl; return EXIT_FAILURE;
	}

	cout << "[ OK ] Done. Press any key to exit." << endl;
	system("pause > nul");
	return EXIT_SUCCESS;

	getchar();
	return 0;
}