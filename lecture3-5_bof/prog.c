#include <stdio.h>


void func(int *a1)
{
    printf(":: &a1's address is 0x%lx \n", (unsigned long)&a1);
}


int main()
{
    int x = 3;
    func(&x);
    return 0;
}
