#include <stdio.h>
#include <stdlib.h>

int main()
{
	unsigned int iYear = 2020;
	char *strCountry = "US";
	float flScrore = 8.5;

	printf(" [+] Next year = %d\n", iYear + 1);
	printf(" [+] Country = %s\n", strCountry);
	printf(" [+] Increment scrore = %.2f\n", flScrore + 1);

	return 0;
}