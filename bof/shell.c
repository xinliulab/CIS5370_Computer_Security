/*
 gcc shell.c -o shell
./shell
*/

#include <unistd.h>

int main() {
    char *argv[] = {"/bin/sh", NULL}; // Provide a valid argument array
    execve(argv[0], argv, NULL);      // Execute /bin/sh with valid arguments
    return 0;  // If execve fails, execution continues
}
