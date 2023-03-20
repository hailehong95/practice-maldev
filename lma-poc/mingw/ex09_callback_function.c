#include <stdio.h>
#include <stdlib.h>

int Add(int a, int b) {
    return a+b;
}

int Mul(int a, int b) {
    return a*b;
}

typedef int(*Calculator)(int, int);

int superCalculator(int a, int b, Calculator ptrCalFunc) {
    return ptrCalFunc(a, b);
}

int main()
{
    printf("[ + ] Local variable Initialized.\n");
    int x = 0, y = 0;

    printf("[ + ] Please input numer:\n");
    printf("number x = ");
    scanf("%d", &x);
    printf("number y = ");
    scanf("%d", &y);

    printf("%d + %d = %d\n", x, y, superCalculator(x, y, Add));
    printf("%d * %d = %d\n", x, y, superCalculator(x, y, Mul));

	return 0;
}