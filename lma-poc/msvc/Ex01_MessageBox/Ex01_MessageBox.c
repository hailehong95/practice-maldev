#include <Windows.h>
#pragma comment(linker, "/SUBSYSTEM:windows /ENTRY:mainCRTStartup")

/*
* Ref:
* 1. https://stackoverflow.com/a/6882500
* 2. https://stackoverflow.com/a/12413270
*/
int main() {
	//FreeConsole(); // For hide console windows.
	MessageBoxA(NULL, "Hello, World!", "Hi!", MB_OK | MB_ICONINFORMATION);
	return 0;
}