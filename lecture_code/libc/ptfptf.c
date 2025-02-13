/*
gcc -g ptfptf.c
objdump -d a.out | less

*/

#include <stdio.h>
#include <stdlib.h>

int main()
{
	printf("%p\n", printf);
    printf("%p\n", exit);
}
