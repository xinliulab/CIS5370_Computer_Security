#include <sys/mman.h>   // for mmap(), munmap()
#include <fcntl.h>      // for open() and file flags like O_RDWR
#include <sys/stat.h>   // for fstat() and struct stat
#include <string.h>     // for memcpy()
#include <stdio.h>      // for printf()
#include <unistd.h>    // for close()

int main()
{
  struct stat st;                        // To store file metadata (e.g., file size)
  char content[20];                      // Buffer to store data read from the file
  char *new_content = "New Content";     // New data to write into the file
  void *map;                             // Pointer to the memory-mapped region

  // Open the file "zzz" in read-write mode
  int f = open("./zzz", O_RDWR);                      

  // Get file size and other info using fstat()
  fstat(f, &st);

  // Map the entire file into memory with both read and write permissions
  // MAP_SHARED means changes to the mapped memory will be written back to the file
  map = mmap(NULL, st.st_size, PROT_READ | PROT_WRITE, MAP_SHARED, f, 0);

  // Copy the first 10 bytes from the mapped memory into the 'content' buffer
  memcpy((void*)content, map, 10);                

  // Print the data read from the file
  printf("read content: %s\n", content);

  // Modify the mapped memory starting from the 6th byte (offset +5)
  // Since MAP_SHARED is used, this change will be reflected in the actual file
  memcpy(map + 5, new_content, strlen(new_content)); 

  // Unmap the memory-mapped region to free resources
  munmap(map, st.st_size);

  // Close the file
  close(f);

  return 0;
}
