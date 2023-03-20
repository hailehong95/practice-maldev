#include <stdio.h>
#include <stdlib.h>

unsigned int MAX_BUFFER = 255;
#define MAX_SCORE 100

int main()
{
	char chr = 'A';
	float score = 95;
	static int level = 69;
	printf(" [+] All in one line: %d %d %c %.2f %d", MAX_BUFFER, MAX_SCORE, chr, score, level);
	return 0;
}