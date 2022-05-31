#include "registry.h"

/*
* Example to used:
	BOOL status;
	DWORD readData;
	PWCHAR readMessage = nullptr;

	status = CreateRegistryKey(HKEY_CURRENT_USER, "Aticleworld"); //create key
	if (status != TRUE) return FALSE;

	status = WriteDwordInRegistry(HKEY_CURRENT_USER, "Aticleworld","date",12082016); //write dword
	if (status != TRUE) return FALSE;

	status = readDwordValueRegistry(HKEY_CURRENT_USER, "Aticleworld", "date", &readData); //read dword
	if (status != TRUE) return FALSE;
	printf("%ld", readData);

	status = writeStringInRegistry(HKEY_CURRENT_USER, "Aticleworld", "Message", "Happy"); //write string
	if (status != TRUE) return FALSE;

	status = readUserInfoFromRegistry(HKEY_CURRENT_USER, "Aticleworld", "Message", &readMessage); //read string
	if (status != TRUE) return FALSE;
	if (readMessage != nullptr) {
		printf(" Message = %S\n", readMessage);
		free(readMessage);
		readMessage = nullptr;
	}
*/

BOOL CreateRegistryKey(HKEY hKeyParent, PWCHAR subkey)
{
	DWORD	dwDisposition; // It verify new key is created or open existing key
	HKEY	hKey;
	DWORD	Ret;

	Ret = RegCreateKeyExW(hKeyParent, subkey, 0, NULL, REG_OPTION_NON_VOLATILE, KEY_ALL_ACCESS, NULL, &hKey, &dwDisposition);
	if (Ret != ERROR_SUCCESS) {
		printf(" [-] Error Opening or Creating new key. Code = %d\n", GetLastError());
		return FALSE;
	}
	//close the key
	RegCloseKey(hKey);
	return TRUE;
}

BOOL WriteDwordToRegistry(HKEY hKeyParent, PWCHAR subkey, PWCHAR valueName, DWORD data)
{
	DWORD	Ret;
	HKEY	hKey;
	// Open the key
	Ret = RegOpenKeyExW(hKeyParent, subkey, 0, KEY_WRITE, &hKey);
	if (Ret == ERROR_SUCCESS) {
		// Set the value in key
		if (ERROR_SUCCESS != RegSetValueExW(hKey, valueName, 0, REG_DWORD, (LPBYTE)&data, sizeof(data))) {
			RegCloseKey(hKey);
			return FALSE;
		}
		// Close the key
		RegCloseKey(hKey);
		return TRUE;
	}
	return FALSE;
}

BOOL WriteStringToRegistry(HKEY hKeyParent, PWCHAR subkey, PWCHAR valueName, PWCHAR strData)
{
	DWORD	Ret;
	HKEY	hKey;
	// Open the key
	Ret = RegOpenKeyExW(hKeyParent, subkey, 0, KEY_WRITE, &hKey);
	if (Ret == ERROR_SUCCESS) {
		if (ERROR_SUCCESS != RegSetValueExW(hKey, valueName, 0, REG_SZ, (LPBYTE)(strData), ((((DWORD)lstrlen(strData) + 1)) * 2))) {
			RegCloseKey(hKey);
			return FALSE;
		}
		RegCloseKey(hKey);
		return TRUE;
	}
	return FALSE;
}

BOOL ReadDwordFromRegistry(HKEY hKeyParent, PWCHAR subkey, PWCHAR valueName, DWORD* readData)
{
	HKEY	hKey;
	DWORD	Ret;
	// Check if the registry exists
	Ret = RegOpenKeyExW(hKeyParent, subkey, 0, KEY_READ, &hKey);
	if (Ret == ERROR_SUCCESS) {
		DWORD data;
		DWORD len = sizeof(DWORD);	// size of data
		Ret = RegQueryValueExW(hKey, valueName, NULL, NULL, (LPBYTE)(&data), &len);
		if (Ret == ERROR_SUCCESS) {
			RegCloseKey(hKey);
			(*readData) = data;
			return TRUE;
		}
		RegCloseKey(hKey);
		return TRUE;
	}
	else {
		return FALSE;
	}
}

BOOL ReadStringFromRegistry(HKEY hKeyParent, PWCHAR subkey, PWCHAR valueName, PWCHAR* readData)
{
	// Read customer infromation from the registry
	HKEY	hKey;
	DWORD	len = TOTAL_BYTES_READ;
	DWORD	readDataLen = len;
	PWCHAR	readBuffer = (PWCHAR)malloc(sizeof(PWCHAR) * len);

	if (readBuffer == NULL)
		return FALSE;
	// Check if the registry exists
	DWORD Ret = RegOpenKeyExW(hKeyParent, subkey, 0, KEY_READ, &hKey);
	if (Ret == ERROR_SUCCESS) {
		Ret = RegQueryValueExW(hKey, valueName, NULL, NULL, (BYTE*)readBuffer, &readDataLen);
		while (Ret == ERROR_MORE_DATA)
		{
			// Get a buffer that is big enough.
			len += OFFSET_BYTES;
			readBuffer = (PWCHAR)realloc(readBuffer, len);
			readDataLen = len;
			Ret = RegQueryValueExW(hKey, valueName, NULL, NULL, (BYTE*)readBuffer, &readDataLen);
		}
		if (Ret != ERROR_SUCCESS) {
			RegCloseKey(hKey);
			return FALSE;;
		}
		*readData = readBuffer;
		RegCloseKey(hKey);
		return TRUE;
	}
	else {
		return FALSE;
	}
}
