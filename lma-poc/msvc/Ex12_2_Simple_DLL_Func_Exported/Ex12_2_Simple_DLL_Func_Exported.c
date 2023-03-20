#include <stdio.h>
#include <stdlib.h>
#include <Windows.h>
#pragma warning(disable : 4996)

#define DllExport   __declspec(dllexport)


/*
** VS Project > Properties > C/C++ > All Option > Precompiled Header > Not Using ...
** VS Project > Properties > Linker > All Option > General Debug Info > No
** Xóa các header file do VS tự động tạo ra.
** Ref:
** 1. MSDN - dllexport, dllimport
	https://docs.microsoft.com/en-us/cpp/cpp/dllexport-dllimport
*/

int sum(int a, int b) {
	return a + b;
}

int mul(int a, int b) {
	return a * b;
}

DllExport void func() {
	char buffer[50] = { 0 };
	int a = 11, b = 9;

	sprintf(buffer, "This is func() exported.\n%d * %d = %d", a, b, mul(a, b));
	MessageBoxA(0, buffer, "func", MB_OK | MB_ICONWARNING);
}

BOOL APIENTRY DllMain(HMODULE hModule, DWORD  ul_reason_for_call, LPVOID lpReserved)
{
	if (ul_reason_for_call == DLL_PROCESS_ATTACH) {
		// Implement malicious code in DllMain Function
		char buffer[50] = { 0 };
		int a = 10, b = 9;

		sprintf(buffer, "This is DllMain.\n%d + %d = %d", a, b, sum(a, b));
		MessageBoxA(0, buffer, "DllMain", MB_OK | MB_ICONWARNING);
	}

	return TRUE;
}