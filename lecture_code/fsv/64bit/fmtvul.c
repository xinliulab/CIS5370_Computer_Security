#include <stdio.h>

void fmtstr(char *str)
{
    void* framep;
    void* ret;

    asm ("mov %%rbp, %0" : "=r" (framep));             
    ret = framep + 8;

    printf("The address of the input array:  %p\n", str);
    printf("The value of the frame pointer:  %p\n", framep);
    printf("The value of the return address: %p\n", (void*)(*((unsigned long long*)ret)));

    printf(str); // The vulnerable place

    printf("The value of the return address: %p\n", (void*)(*((unsigned long long*)ret)));

}

int main(int argc, char **argv)
{
    FILE *badfile;
    char str[200];

    badfile = fopen("badfile", "rb");
    fread(str, sizeof(char), 200, badfile);
    fmtstr(str);

    return 0;
}
