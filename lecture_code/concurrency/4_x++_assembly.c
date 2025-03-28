/*
Steps to compile and run this code:

1.  Open the terminal and navigate to the folder containing this code:
    cd /path/to/your/code

2.  Compile the code using gcc:
    gcc ex_5_asm_lock.c

3.  Run the executable:
    ./a.out

4.  Use `objdump` to inspect the generated assembly instructions:
    objdump -d a.out
    (This command disassembles the executable so you can observe the actual assembly instructions that the CPU will execute. You can see how the high-level code is broken down into assembly.)


5.  In this version, we replaced the `x++` with an assembly language instruction using the `lock` prefix to ensure atomicity. 
    However, errors still occur when the CPU reorders or further breaks down these instructions.

6.  This demonstrates that even low-level assembly instructions can suffer from race conditions in a multi-threaded environment, emphasizing the need for proper synchronization mechanisms like mutexes.
*/


#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>

#define N 100
long x = 0;


void *Tsum(void *arg) {
    for (int i = 0; i < N; i++) { 
        asm volatile("addq $1, %0": "+m"(x)); 
        // asm volatile("lock addq $1, %0": "+m"(x)); 
    }
    return NULL;
}

int main() {
    pthread_t thread1, thread2;


    pthread_create(&thread1, NULL, Tsum, NULL);
    pthread_create(&thread2, NULL, Tsum, NULL);

    pthread_join(thread1, NULL);
    pthread_join(thread2, NULL);


    printf("x = %ld\n", x);

    return 0;
}
