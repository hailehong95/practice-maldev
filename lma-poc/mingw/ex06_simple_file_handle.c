#include <stdio.h>
#include <string.h>
#include <Windows.h>

int main()
{
	TCHAR filePath[MAX_PATH];

	// Current working directory
	GetCurrentDirectoryA(MAX_PATH, filePath);

	// Format path to save file
	wsprintfA(filePath, TEXT("%s\\simple_file_handle.txt"), filePath);

	printf(" [+] Win32 API Example: CreateFile, WriteFile.\n");
	HANDLE hFile = CreateFileA(filePath, GENERIC_WRITE, FILE_SHARE_READ, NULL, CREATE_NEW, FILE_ATTRIBUTE_NORMAL, NULL);

	if (hFile == INVALID_HANDLE_VALUE) {
		printf(" [-] Invalid handle file.\n"); exit(2);
	}

	char buffer[] = "Lorem Ipsum is simply dummy text of the printing and typesetting industry.";
	DWORD bytesWritten;
	int buffer_size = strlen(buffer);
	WriteFile(hFile, buffer, buffer_size, &bytesWritten, NULL);
	CloseHandle(hFile);

	return 0;
}