// testMessageBox.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
#include <iostream>
#include <Windows.h>

int main()
{
	MessageBoxA(0, "Hello Friend, I come from testMessageBox", "Process Injection Techniques", MB_ICONWARNING);
	return 0;
}