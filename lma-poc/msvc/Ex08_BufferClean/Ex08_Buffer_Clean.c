#include <stdio.h>
#include <stdlib.h>
#pragma warning(disable : 4996)

int main()
{
	char name[25], country[20];
	int age, c;

	printf("Name = ");
	gets(name);

	printf("Age = ");
	scanf("%d", &age);

	while ((c = getchar()) != '\n' && c != EOF) {}

	printf("Country = ");
	gets(country);

	printf("Name: %s, Country: %s, Age: %d\n", name, country, age);

	return 0;
}
