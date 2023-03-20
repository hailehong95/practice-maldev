#include <Windows.h>

/*
** Visual Studio Configuration Precompiled Header
** Project > Properties > C/C++ > All Option > Precompiled Header > Not Using ...
** => Xóa hết các file header .h và các file phụ khác do Visual Studio sinh ra. Chỉ để dllmain.cpp hoặc dllmain.c
*/

BOOL APIENTRY DllMain(HMODULE hModule, DWORD  ul_reason_for_call, LPVOID lpReserved)
{
	if (ul_reason_for_call == DLL_PROCESS_ATTACH) {
		MessageBoxA(0, "This is DllMain.", "Simple DLL", MB_OK | MB_ICONWARNING);
	}

	return TRUE;
}