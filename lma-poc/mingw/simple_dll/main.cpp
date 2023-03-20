#include "main.h"
#include <stdio.h>

void DLL_EXPORT FooFun0()
{
    MessageBoxA(0, "No Args!", "FooFun0", MB_OK | MB_ICONINFORMATION);
}

void DLL_EXPORT FooFun1(char *first)
{
    MessageBoxA(0, "One Args!", "FooFun1", MB_OK | MB_ICONINFORMATION);
}

void DLL_EXPORT FooFun2(int first, int last)
{
    int a, b, c;
    a = 25;
    b = 75;
    c = a + b;
    char buff[100];
    sprintf(buff, "Three Args!\n%d + %d = %d", a, b, c);
    MessageBoxA(0, buff, "FooFun2", MB_OK | MB_ICONINFORMATION);
}

extern "C" DLL_EXPORT BOOL APIENTRY DllMain(HINSTANCE hinstDLL, DWORD fdwReason, LPVOID lpvReserved)
{
    int number1, number2, number3;
    number1 = 8;
    number2 = 2;
    number3 = number1 + number2;
    char buff[100];
    sprintf(buff, "DllMain Function\n%d + %d = %d", number1, number2, number3);

    MessageBoxA(0, buff, "DllMain", MB_OK | MB_ICONINFORMATION);

    switch (fdwReason)
    {
        case DLL_PROCESS_ATTACH:
            // attach to process
            // return FALSE to fail DLL load
            break;

        case DLL_PROCESS_DETACH:
            // detach from process
            break;

        case DLL_THREAD_ATTACH:
            // attach to thread
            break;

        case DLL_THREAD_DETACH:
            // detach from thread
            break;
    }
    return TRUE; // succesful
}
