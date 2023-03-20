#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#pragma warning(disable : 4996)

/*
** Name : SFET - Simple File Encryption Tool
** Version : v0.1 - Beta version
** Crypto Algorithm support: XOR (AES, RSA, Hashing,.v.v.. is comming)
*/

void printf_usage(char *str)
{
	printf(" [+] Usage: %s <file_in.bin> <enc_key> <file_out.bin>\n", str);
}

int isValidXorKey(const char *key)
{
	// xor key must be a numberic
	int len = strlen(key);
	for (int i = 0; i < len; i++)
		if (key[i] < '0' || key[i] > '9')
			return 0;
	return 1;
}

unsigned long long getSizeOfFile(char *filename)
{
	FILE *fptr = NULL;
	unsigned long long sizeOfFile = 0;
	if ((fptr = fopen(filename, "rb")) == NULL) {
		printf(" [-] Error! Opening file \'%s\' for calculating size.\n", filename); exit(2);
	}

	fseek(fptr, 0L, SEEK_END);
	sizeOfFile = ftell(fptr);
	fclose(fptr);
	return sizeOfFile;
}

void readFileBinary(char *buffer, char *filename, unsigned long long filesize)
{
	FILE *fptr = NULL;
	unsigned long long numberOfBytes = 0;

	if ((fptr = fopen(filename, "rb")) == NULL) {
		printf(" [-] Error! Opening file \'%s\' for reading.\n", filename); exit(2);
	}

	numberOfBytes = fread(buffer, 1, filesize, fptr);
	if (numberOfBytes != filesize) {
		printf(" [-] Error! Reading file\n"); exit(3);
	}

	fclose(fptr);
}

void writeFileBinary(char *buffer, char *filename, unsigned long long filesize)
{
	FILE *fptr = NULL;
	unsigned long long numberOfBytes = 0;

	if ((fptr = fopen(filename, "wb")) == NULL) {
		printf(" [-] Error! Opening file \'%s\' for writing.\n", filename); exit(2);
	}

	numberOfBytes = fwrite(buffer, 1, filesize, fptr);
	if (numberOfBytes != filesize) {
		printf(" [-] Error! Writing file\n"); exit(4);
	}

	fclose(fptr);
}

void bufferEncoding(char *buffer, int xorKey, unsigned long long filesize)
{
	unsigned long long i;
	for (i = 0; i < filesize; i++)
		buffer[i] ^= xorKey;
}

int main(int argc, char const *argv[])
{
	// Get program name
	char progName[50];
	char *lastToken = strrchr(argv[0], '\\');

	if (lastToken != NULL) strcpy(progName, lastToken + 1);
	else strcpy(progName, argv[0]);

	// Check arguments
	if (argc != 4) {
		printf_usage(progName); exit(5);
	}

	char fileIn[50], fileOut[50];
	int xorKey = 0;

	// Parsing arguments
	strcpy(fileIn, argv[1]);
	if (isValidXorKey(argv[2])) {
		xorKey = atoi(argv[2]);
	} else {
		printf(" [-] Error! Invalid key. The encrypt key must be numberic!\n"); exit(5);
	}
	strcpy(fileOut, argv[3]);

	char *buffer = NULL;
	unsigned long long sizeOfFile = 0;

	// Obtain file size
	sizeOfFile = getSizeOfFile(fileIn);

	// Allocate memory to contain the whole file
	buffer = (char *)malloc(sizeOfFile * sizeof(char));
	if (buffer == NULL) {
		printf(" [-] Error! Memory not allocated.\n"); exit(3);
	}

	// Reading file to buffer
	readFileBinary(buffer, fileIn, sizeOfFile);

	// Encoding payload
	bufferEncoding(buffer, xorKey, sizeOfFile);

	// Write buffer to new file
	writeFileBinary(buffer, fileOut, sizeOfFile);

	// Terminate
	free(buffer);

	return 0;
}