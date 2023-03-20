#pragma once
#include <stdio.h>
#include <stdlib.h>
#include <Windows.h>

#define	TOTAL_BYTES_READ	1024
#define	OFFSET_BYTES		1024

BOOL CreateRegistryKey(HKEY hKeyParent, PWCHAR subkey);
BOOL WriteDwordToRegistry(HKEY hKeyParent, PWCHAR subkey, PWCHAR valueName, DWORD data);
BOOL WriteStringToRegistry(HKEY hKeyParent, PWCHAR subkey, PWCHAR valueName, PWCHAR strData);
BOOL ReadDwordFromRegistry(HKEY hKeyParent, PWCHAR subkey, PWCHAR valueName, DWORD *readData);
BOOL ReadStringFromRegistry(HKEY hKeyParent, PWCHAR subkey, PWCHAR valueName, PWCHAR *readData);