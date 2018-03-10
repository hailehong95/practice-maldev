#include <iostream>
#include <stdlib.h>
#include <Windows.h>
#include <iomanip>
#include <winnt.h>

using namespace std;

void ShowOptions();

int main(int argc, char const *argv[])
{
	HANDLE hFile, hMapObject;
	LPVOID lpBase;
	PIMAGE_DOS_HEADER pDosHeader;
	PIMAGE_NT_HEADERS pNtHeader; // 32 or 64
	IMAGE_FILE_HEADER fileHeader;
	IMAGE_OPTIONAL_HEADER opHeader; // 32 or 64
	PIMAGE_SECTION_HEADER pSectionHeader, pSecHeaderForExport;

	// Command Line Arguments
	if (argc > 2)
	{
		hFile = CreateFileA(argv[2], GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
		if (hFile == INVALID_HANDLE_VALUE)
		{
			cout << "ERROR : Could not open the file specified" << endl;
			ExitProcess(69);
		}

		// Mapping Given EXE file to Memory
		hMapObject = CreateFileMapping(hFile, NULL, PAGE_READONLY, 0, 0, NULL);
		lpBase = MapViewOfFile(hMapObject, FILE_MAP_READ, 0, 0, 0);

		if (strcmp(argv[1], "-d") == 0)
		{
			// Get the DOS Header Base
			pDosHeader = (PIMAGE_DOS_HEADER)lpBase; // 0x04000000

													// Check for Valid DOS file
			if (pDosHeader->e_magic == IMAGE_DOS_SIGNATURE) // IMAGE_DOS_SIGNATURE = 0x5A4D = MZ
			{
				//Dump the Dos Header info
				cout << "Valid Dos Exe File" << endl;
				cout << "------------------" << endl;
				cout << "Dumping DOS Header Info" << endl;
				cout << "-----------------------" << endl;

				cout << "e_magic\t\t\t" << hex << "0x" << pDosHeader->e_magic << endl;
				cout << "e_cblp\t\t\t" << hex << "0x" << pDosHeader->e_cblp << endl;
				cout << "e_cp\t\t\t" << hex << "0x" << pDosHeader->e_cp << endl;
				cout << "e_crlc\t\t\t" << hex << "0x" << pDosHeader->e_crlc << endl;
				cout << "e_cparhdr\t\t" << hex << "0x" << pDosHeader->e_cparhdr << endl;
				cout << "e_sp\t\t\t" << hex << "0x" << pDosHeader->e_sp << endl;
				cout << "e_csum\t\t\t" << hex << "0x" << pDosHeader->e_csum << endl;
				cout << "e_ip\t\t\t" << hex << "0x" << pDosHeader->e_ip << endl;
				cout << "e_lfanew\t\t" << hex << "0x" << pDosHeader->e_lfanew << endl;
				cout << endl;
			}
		}

		if (strcmp(argv[1], "-c") == 0)
		{
			// Get the DOS Header Base
			pDosHeader = (PIMAGE_DOS_HEADER)lpBase; // 0x04000000
													// Get the Base of NT Header(PE Header)  = dosHeader + RVA address of PE header
			pNtHeader = (PIMAGE_NT_HEADERS)((DWORD)(pDosHeader)+(pDosHeader->e_lfanew));

			// Identify for valid PE file
			if (pNtHeader->Signature == IMAGE_NT_SIGNATURE)
			{
				cout << "Valid NT Header File" << endl;
				cout << "--------------------" << endl;
				cout << "Dumping NT Header Info" << endl;
				cout << "-----------------------" << endl;

				cout << "Signature\t\t" << hex << "0x" << pNtHeader->Signature << " (PE)" << endl;
				// Get the IMAGE FILE HEADER Structure
				fileHeader = pNtHeader->FileHeader;

				// Determine Machine Architechture
				cout << "Machine\t\t\t";
				switch (fileHeader.Machine)
				{
				case 0x0:    cout << "All" << endl; break;
				case 0x14c:  cout << "Intel i386" << endl; break;
				case 0x162:  cout << "R3000 - MIPS" << endl; break;
				case 0x166:  cout << "R4000 - MIPS" << endl; break;
				case 0x1c0:  cout << "ARM" << endl; break;
				case 0x1f0:  cout << "IBM PowerPC" << endl; break;
				case 0x200:  cout << "Intel 64" << endl; break;
				case 0x14d:  cout << "Intel i860" << endl; break;
				case 0x8664: cout << "AMD64 (K8)" << endl; break;
				default:     cout << "Not Found" << endl; break;
				}

				cout << "NumberOfSections\t" << hex << "0x" << fileHeader.NumberOfSections << endl;
				cout << "TimeDateStamp\t\t" << hex << "0x" << fileHeader.TimeDateStamp << endl;
				cout << "PointerToSymbolTable\t" << hex << "0x" << fileHeader.PointerToSymbolTable << endl;
				cout << "NumberOfSymbols\t\t" << hex << "0x" << fileHeader.NumberOfSymbols << endl;
				cout << "SizeOfOptionalHeader\t" << hex << "0x" << fileHeader.SizeOfOptionalHeader << endl;

				// Determine the characteristics of the given file
				cout << "Characteristics\t\t" << hex << "0x" << fileHeader.Characteristics << " (";
				//cout << "Characteristics\t\t";
				if ((fileHeader.Characteristics & 0x0002) == 0x0002) cout << "Executable, ";
				if ((fileHeader.Characteristics & 0x2000) == 0x2000) cout << "Dll, ";
				if ((fileHeader.Characteristics & 0x1000) == 0x1000) cout << "System, ";
				if ((fileHeader.Characteristics & 0x0020) == 0x0020) cout << "App can Handle > 2GB Addr Space, ";
				if ((fileHeader.Characteristics & 0x4000) == 0x4000) cout << "App runs only in Uniprocessor, ";
				if ((fileHeader.Characteristics & 0x0100) == 0x0100) cout << "32-bits, ";
				cout << ")" << endl;
				cout << endl;
			}
		}

		if (strcmp(argv[1], "-p") == 0)
		{
			// Get the DOS Header Base
			pDosHeader = (PIMAGE_DOS_HEADER)lpBase; // 0x04000000
													// Get the Base of NT Header(PE Header)  = dosHeader + RVA address of PE header
			pNtHeader = (PIMAGE_NT_HEADERS)((DWORD)(pDosHeader)+(pDosHeader->e_lfanew));
			// Info about Optional Header
			opHeader = pNtHeader->OptionalHeader;

			cout << "Valid NT Header File" << endl;
			cout << "--------------------" << endl;
			cout << "Dumping PE Optional Header Info" << endl;
			cout << "-------------------------------" << endl;

			cout << "AddressOfEntryPoint\t" << hex << "0x" << opHeader.AddressOfEntryPoint << endl;
			cout << "CheckSum\t\t" << hex << "0x" << opHeader.CheckSum << endl;
			cout << "ImageBase\t\t" << hex << "0x" << opHeader.ImageBase << endl;
			cout << "FileAlignment\t\t" << hex << "0x" << opHeader.FileAlignment << endl;
			cout << "SizeOfImage\t\t" << hex << "0x" << opHeader.SizeOfImage << endl;
			cout << "SectionAlignment\t" << hex << "0x" << opHeader.SectionAlignment << endl;
			cout << endl;
		}

		if (strcmp(argv[1], "-s") == 0)
		{
			// Get the DOS Header Base
			pDosHeader = (PIMAGE_DOS_HEADER)lpBase; // 0x04000000
			pNtHeader = (PIMAGE_NT_HEADERS)((DWORD)(pDosHeader)+(pDosHeader->e_lfanew));

			cout << "Dumping Section Header Info" << endl;
			cout << "---------------------------" << endl;
			cout << endl;

			int i;
			cout << " Name\t  VirtualSize\t VirtualAddress(RVA)\t   RawSize\t RawAddress\t Characteristics" << endl;
			for (pSectionHeader = IMAGE_FIRST_SECTION(pNtHeader), i = 0; i < pNtHeader->FileHeader.NumberOfSections; i++, pSectionHeader++)
			{
				cout << pSectionHeader->Name;
				cout << "\t    " << hex << "0x" << pSectionHeader->Misc.VirtualSize;
				cout << "\t      " << hex << "0x" << pSectionHeader->VirtualAddress;
				cout << "\t\t    " << hex << "0x" << pSectionHeader->SizeOfRawData;
				cout << "\t   " << hex << "0x" << pSectionHeader->PointerToRawData;
				cout << "\t    " << hex << "0x" << pSectionHeader->Characteristics;
				cout << endl;
			}
			cout << endl;
		}

		if (strcmp(argv[1], "-i") == 0)
		{
			// Check PE32 or PE64
			pDosHeader = (PIMAGE_DOS_HEADER)lpBase;
			pNtHeader = (PIMAGE_NT_HEADERS)((DWORD)(pDosHeader)+(pDosHeader->e_lfanew));
			opHeader = pNtHeader->OptionalHeader;
			if (opHeader.Magic != 0x10b)
				return 69;

			// Get the DOS Header Base
			pDosHeader = (PIMAGE_DOS_HEADER)lpBase;

			// Import
			DWORD dwImportDirectoryVA, dwSectionCount, dwSection = 0, dwRawOffset;
			PIMAGE_IMPORT_DESCRIPTOR pImportDescriptor; // pExportDescriptor
			PIMAGE_THUNK_DATA32 pThunkData; // 32 or 64

			pNtHeader = (PIMAGE_NT_HEADERS)((DWORD)lpBase + pDosHeader->e_lfanew);
			dwSectionCount = pNtHeader->FileHeader.NumberOfSections;
			dwImportDirectoryVA = pNtHeader->OptionalHeader.DataDirectory[1].VirtualAddress; // DataDirectory[0]=> Export
			pSectionHeader = (PIMAGE_SECTION_HEADER)((DWORD)pNtHeader + sizeof(IMAGE_NT_HEADERS32));

			cout << "Dumping Import Directory Info" << endl;
			cout << "-----------------------------" << endl;

			for (; dwSection < dwSectionCount && pSectionHeader->VirtualAddress <= dwImportDirectoryVA; pSectionHeader++, dwSection++);
			pSectionHeader--;
			dwRawOffset = (DWORD)lpBase + pSectionHeader->PointerToRawData;
			pImportDescriptor = (PIMAGE_IMPORT_DESCRIPTOR)(dwRawOffset + (dwImportDirectoryVA - pSectionHeader->VirtualAddress));

			//PULONG compare = (PULONG)(HIBYTE(HIWORD(IMAGE_ORDINAL_FLAG32)));
			
			for (int j = 1; pImportDescriptor->Name != 0; pImportDescriptor++, j++)
			{
				DWORD TenDLL = dwRawOffset + (pImportDescriptor->Name - pSectionHeader->VirtualAddress);
				printf("\nDLL Name %d: %s\n", j, TenDLL);

				pThunkData = (PIMAGE_THUNK_DATA32)(dwRawOffset + (pImportDescriptor->FirstThunk - pSectionHeader->VirtualAddress));

				for (int i = 1; pThunkData->u1.AddressOfData != 0; pThunkData++, i++)
				{
					DWORD TenHam = (dwRawOffset + (pThunkData->u1.AddressOfData - pSectionHeader->VirtualAddress + 2));
					printf("\tFunction %d:\t%s\n", i, TenHam);

				}
			}

		}

		if (strcmp(argv[1], "-e") == 0)
		{
			// Check PE32 or PE64
			pDosHeader = (PIMAGE_DOS_HEADER)lpBase;
			pNtHeader = (PIMAGE_NT_HEADERS)((DWORD)(pDosHeader)+(pDosHeader->e_lfanew));
			opHeader = pNtHeader->OptionalHeader;
			if (opHeader.Magic != 0x10b)
				return 96;

			// Get the DOS Header Base
			pDosHeader = (PIMAGE_DOS_HEADER)lpBase;

			// Export
			DWORD dwSectionCount, dwExportDirectoryVA, dwSection = 0, dwRawOffset;
			PIMAGE_EXPORT_DIRECTORY pExportDirectory;

			pNtHeader = (PIMAGE_NT_HEADERS32)((DWORD)lpBase + pDosHeader->e_lfanew);

			pSectionHeader = (PIMAGE_SECTION_HEADER)((DWORD)pNtHeader + sizeof(IMAGE_NT_HEADERS32));
			dwSectionCount = pNtHeader->FileHeader.NumberOfSections;
			//dwImportDirectoryVA = pNtHeader->OptionalHeader.DataDirectory[1].VirtualAddress; // DataDirectory[0]=> Export
			dwExportDirectoryVA = pNtHeader->OptionalHeader.DataDirectory[0].VirtualAddress;

			for (; dwSection < dwSectionCount && pSectionHeader->VirtualAddress <= dwExportDirectoryVA; pSectionHeader++, dwSection++);
			pSectionHeader--;
			pSecHeaderForExport = pSectionHeader;
			//dwRawOffset = (DWORD_PTR)lpBase + pSecHeaderForExport->PointerToRawData;
			dwRawOffset = (DWORD)lpBase + pSecHeaderForExport->PointerToRawData;
			pExportDirectory = (PIMAGE_EXPORT_DIRECTORY)(dwRawOffset + (dwExportDirectoryVA - pSecHeaderForExport->VirtualAddress));

			if (pExportDirectory->AddressOfFunctions != NULL)
			{
				cout << "Dumping Export Directory Info" << endl;
				cout << "-----------------------------" << endl;

				PULONG Name = (PULONG)((PUCHAR)dwRawOffset + (pExportDirectory->AddressOfNames - pSecHeaderForExport->VirtualAddress));
				LPSTR functionName;
				for (int i = 0; i < pExportDirectory->NumberOfNames; i++)
				{
					functionName = dwRawOffset + ((char*)Name[i] - pSecHeaderForExport->VirtualAddress);
					printf("\tFunction %d:\t%s\n", i + 1, functionName);
					//printf("\n%-36s%s", "Fucntion Exported by Name: ", functionName);
				}

			}

		}

		if (strcmp(argv[1], "-h") == 0)
		{
			ShowOptions();
		}

		UnmapViewOfFile(lpBase);
		CloseHandle(hMapObject);
		CloseHandle(hFile);

	}
	else
		ShowOptions();

	return 0;
}

void ShowOptions()
{
	cout << "\nPE Mini Tools v1.0" << endl;
	cout << "Copyright (C) 2017 Le Hong Hai" << endl;
	cout << endl;
	cout << "Usage:\t PEMiniTools.exe [-Option] [Target]" << endl;
	cout << "Option:" << endl;
	cout << "  -h\t Help" << endl;
	cout << "  -c\t Check PE File" << endl;
	cout << "  -d\t DOS Header" << endl;
	cout << "  -p\t PE Optional Header" << endl;
	cout << "  -s\t Section Header" << endl;
	cout << "  -i\t Import Directory" << endl;
	cout << "  -e\t Export Directory" << endl;
	cout << endl;
}