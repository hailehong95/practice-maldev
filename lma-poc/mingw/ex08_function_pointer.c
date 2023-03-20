#include <stdio.h>
#include <stdlib.h>

int Add(int a, int b) {
    return a+b;
}

int Mul(int a, int b) {
    return a*b;
}

int main()
{
    typedef int(*ptrFunc)(int, int);
    
    printf("[ + ] Initialize local variable and function pointer!\n");
    ptrFunc ptrAdd = NULL, ptrMul = NULL;
    int x = 0, y = 0;

    printf("[ + ] Assigned Ptr.\n");
    ptrAdd = &Add;
    ptrMul = &Mul;

    printf("[ + ] Please input numer:\n");
    printf("number x = ");
    scanf("%d", &x);
    printf("number y = ");
    scanf("%d", &y);

    printf("%d + %d = %d\n", x, y, ptrAdd(x, y));
    printf("%d * %d = %d\n", x, y, ptrMul(x, y));

	return 0;
}