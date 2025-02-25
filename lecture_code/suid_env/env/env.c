#include <stdio.h>

// A mysteriously defined symbol.
// Someone must define it elsewhere.
extern char **environ;

// Like this even more mysterious one.
// "end" can be of any type.
extern void *****************************end;

int main(int argc, char *argv[], char *envp[]) {
    printf("%p\n", environ);
    printf("%p\n", envp);

    /*
    for (char **env = environ; *env; env++) {
        // key=value
        printf("%s\n", *env);
    }
    */
}
