#include <Windows.h>
// For hide console windows: -mwindows
// $ gcc.exe .\ex00_messagebox.c -mwindows -o .\bin\ex00_messagebox
int main() {
    MessageBoxA(NULL, "Hello, World!", "Hi!", MB_OK | MB_ICONINFORMATION);
    return 0;
}