// dllmain.cpp : Defines the entry point for the DLL application.
#include "stdafx.h"
#include <Windows.h>

BOOL APIENTRY DllMain(HMODULE hModule,
	DWORD  ul_reason_for_call,
	LPVOID lpReserved
)
{
	if (ul_reason_for_call == DLL_PROCESS_ATTACH)
		MessageBox(0, L"Hello Friend, I come from testLibs.dll", L"Process Injection Techniques", MB_ICONWARNING);

	return TRUE;
}

extern "C" __declspec(dllexport) int myFunction()
{
	MessageBox(0, L"Hi Friend!, I'm myFunction()! I come from testLibs.dll", L"Process Injection Techniques", MB_ICONWARNING);
	return 1;
}