#include <stdio.h>

int main(){

	printf("%p\n",main);

	int x = *(int *)main;
	printf("%x\n",x);

	// int *p = (void*) 0x12345678LL;
}
