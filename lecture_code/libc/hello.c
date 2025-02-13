/*
gcc hello.c
ls -l
file a.out

gcc -staic.c
ls -l
file a.out

gcc -c hello.c
readelf -s hello.o

*/

#include <stdio.h>

int main()
{
	printf("Hello!\n");
}
