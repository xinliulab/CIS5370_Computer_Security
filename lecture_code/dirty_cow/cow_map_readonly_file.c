#include <stdio.h>         // for printf()
#include <sys/mman.h>      // for mmap(), madvise()
#include <fcntl.h>         // for open()
#include <unistd.h>        // for close(), lseek(), write()
#include <string.h>        // for memcpy(), strlen()
#include <sys/stat.h>      // for fstat(), struct stat
#include <stdlib.h>        // for exit()

int main()
{
    // The content we want to write into the mapped memory
    char *content = "**New content**";

    // Buffer to read back and print memory content
    char buffer[30];

    // Used to store file size and metadata
    struct stat st;

    // Pointer to mapped memory
    void *map;

    // Open the file "zzz" in read-only mode
    int f = open("zzz", O_RDONLY);
    if (f < 0) {
        perror("open");
        exit(1);
    }

    // Get file size and information
    if (fstat(f, &st) == -1) {
        perror("fstat");
        close(f);
        exit(1);
    }

    // Map the file to memory: PROT_READ + MAP_PRIVATE
    // Meaning: read-only and private copy (Copy-on-Write)
    map = mmap(NULL, st.st_size, PROT_READ, MAP_PRIVATE, f, 0);
    if (map == MAP_FAILED) {
        perror("mmap");
        close(f);
        exit(1);
    }

    // Open the special memory file for the current process
    int fm = open("/proc/self/mem", O_RDWR);
    if (fm < 0) {
        perror("open /proc/self/mem");
        munmap(map, st.st_size);
        close(f);
        exit(1);
    }

    // Move the file pointer in /proc/self/mem to the 6th byte in the mapped memory
    if (lseek(fm, (off_t) map + 5, SEEK_SET) == -1) {
        perror("lseek");
        close(fm);
        munmap(map, st.st_size);
        close(f);
        exit(1);
    }

    // Write the new content into the mapped memory via /proc/self/mem
    write(fm, content, strlen(content));

    // Read back the memory content to check if the write worked
    memcpy(buffer, map, 29);
    buffer[29] = '\0';  // Null-terminate for safety
    printf("Content after write: %s\n", buffer);

    // Discard the copied memory using madvise()
    // This simulates reverting to the original file content
    madvise(map, st.st_size, MADV_DONTNEED);

    // Read again after madvise to see if changes were discarded
    memcpy(buffer, map, 29);
    buffer[29] = '\0';
    printf("Content after madvise: %s\n", buffer);

    // Clean up
    close(fm);
    munmap(map, st.st_size);
    close(f);

    return 0;
}
