/* This program has a buffer overflow vulnerability. */
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

int foo(char *str)
{
    char buffer[100];

    /* The following statement has a buffer overflow problem */
    strcpy(buffer, str);                      

    return 1;
}

int main(int argc, char **argv)
{
    char str[700];
    FILE *badfile;

    badfile = fopen("badfile", "r");
    fread(str, sizeof(char), 700, badfile);    
    foo(str);

    printf("Returned Properly\n");
    return 1;
}
