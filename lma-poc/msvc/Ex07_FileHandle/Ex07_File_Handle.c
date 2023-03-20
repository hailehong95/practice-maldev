#include <stdio.h>
#include <tchar.h>
#include <stdlib.h>
#include <string.h>
#include <Windows.h>

/*
** Ref:
** 1. Opening a File for Reading or Writing
**    https://docs.microsoft.com/en-us/windows/win32/fileio/opening-a-file-for-reading-or-writing
** 2. Check the file-size without opening file in C++?
**    https://stackoverflow.com/a/8991228
** 3. MSDN
**    https://docs.microsoft.com/en-us/windows/win32/api/fileapi/nf-fileapi-createfilea
**    https://docs.microsoft.com/en-us/windows/win32/api/fileapi/nf-fileapi-writefile
**    https://docs.microsoft.com/en-us/windows/win32/api/fileapi/nf-fileapi-readfile
** 4. Which ReadFile parameter in this code is incorrect? (Error code 87)
**    https://stackoverflow.com/a/36755313
*/

void MyWriteFile(TCHAR *filePath, char *dataBuffer)
{
	HANDLE	hFileWriter;
	DWORD	dwBytesToWrite = (DWORD)strlen(dataBuffer);
	DWORD	dwBytesWritten = 0;
	BOOL	bErrorFlag = FALSE;

	hFileWriter = CreateFile(
		filePath,				// name of the write
		GENERIC_WRITE,			// open for writing
		NULL,					// do not share
		NULL,					// default security
		CREATE_NEW,				// create new file only
		FILE_ATTRIBUTE_NORMAL,	// normal file
		NULL);					// no attr. template

	if (hFileWriter == INVALID_HANDLE_VALUE) {
		printf(" [-] Terminal failure: Unable to open file \"%s\" for write. Error Code = %d\n", filePath, GetLastError());
		return;
	}

	printf(" [+] Writing %d bytes to \"%s\".\n", dwBytesToWrite, filePath);

	bErrorFlag = WriteFile(
		hFileWriter,		// open file handle
		dataBuffer,			// start of data to write
		dwBytesToWrite,		// number of bytes to write
		&dwBytesWritten,	// number of bytes that were written
		NULL);				// no overlapped structure
	if (FALSE == bErrorFlag) {
		printf(" [-] Terminal failure: Unable to write to file. Error Code = %d\n", GetLastError());
	}
	else {
		if (dwBytesWritten != dwBytesToWrite) {
			// Mặc dù WriteFile có ghi thành công (trả về TRUE). Nhưng:
			// Số byte muốn ghi ban đầu khác với số byte hàm WriteFile đã ghi.
			// Trường hợp này xảy ra có thể do bất đồng bộ trong quá trình ghi file.
			printf(" [-] Error: dwBytesWritten != dwBytesToWrite. Error Code = %d\n", GetLastError());
		}
		else {
			printf(" [+] Wrote %d bytes to \"%s\" successfully.\n", dwBytesWritten, filePath);
		}
	}
	CloseHandle(hFileWriter);
}

void MyReadFile(TCHAR *filePath, char *dataBuffer, DWORD dwBytesToRead)
{
	HANDLE	hFileReader;
	DWORD	dwBytesRead = 0;
	BOOL	bErrorFlag = FALSE;

	hFileReader = CreateFile(
		filePath,				// file to open
		GENERIC_READ,			// open for reading
		FILE_SHARE_READ,		// share for reading
		NULL,					// default security
		OPEN_EXISTING,			// existing file only
		FILE_ATTRIBUTE_NORMAL,	// normal file
		NULL);					// no attr. template
	
	if (hFileReader == INVALID_HANDLE_VALUE) {
		printf(" [-] Terminal failure: Unable to open file \"%s\" for read. Error Code = %d\n", filePath, GetLastError());
		return;
	}

	printf(" [+] Reading %d bytes from \"%s\".\n", dwBytesToRead, filePath);

	bErrorFlag = ReadFile(
		hFileReader,		// open file handle
		dataBuffer,			// start of data to read
		dwBytesToRead,		// number of bytes to read
		&dwBytesRead,		// number of bytes that were readed
		NULL);

	if (FALSE == bErrorFlag)
	{
		printf(" [-] Terminal failure: Unable to read from file. Error Code = %d\n", GetLastError());
		CloseHandle(hFileReader);
		return;
	}
	SleepEx(1000, TRUE);

	if (dwBytesRead > 0) {
		// Trường hợp đọc file thành công. Nhưng có thể:
		// Đọc được hết file.
		// Không đọc đc trọn vẹn file.
		printf(" [+] Data read from \"%s\" (%d/%d bytes).\n", filePath, dwBytesRead, dwBytesToRead);
		// show data or return to buffer
	}
	else if (dwBytesRead == 0) {
		printf(" [-] No data read from file \"%s\"\n", filePath);
	}
	else {
		printf(" [-] Unexpected value for dwBytesRead.\n");
	}

	CloseHandle(hFileReader);
}

__int64 MyGetFileSize(char *filePath)
{
	LARGE_INTEGER sizeOfFile;
	HANDLE hFileGetSize;

	hFileGetSize = CreateFile(
		filePath,
		GENERIC_READ,
		FILE_SHARE_READ | FILE_SHARE_WRITE,
		NULL,
		OPEN_EXISTING,
		FILE_ATTRIBUTE_NORMAL,
		NULL);

	if (hFileGetSize == INVALID_HANDLE_VALUE)
	{
		return -1; // error condition, could call GetLastError to find out more
	}

	if (!GetFileSizeEx(hFileGetSize, &sizeOfFile))
	{
		CloseHandle(hFileGetSize);
		return -1; // error condition, could call GetLastError to find out more
	}

	CloseHandle(hFileGetSize);

	return sizeOfFile.QuadPart;
}

int main()
{
	TCHAR filePath[MAX_PATH];
	__int64 sizeOfFile = 0;

	GetCurrentDirectoryA(MAX_PATH, filePath);
	wsprintfA(filePath, TEXT("%s\\Ex07_File_Handle.txt"), filePath);

	char dataBuffer[] = "Lorem Ipsum is simply dummy text of the printing and typesetting industry.";
	MyWriteFile(filePath, dataBuffer);

	sizeOfFile = MyGetFileSize(filePath);
	char *newDataBuffer = (char *)malloc(sizeOfFile * sizeof(char));
	MyReadFile(filePath, newDataBuffer, (DWORD)sizeOfFile);
	printf(" [+] Data: %s\n", newDataBuffer);
	free(newDataBuffer);

	return 0;
}