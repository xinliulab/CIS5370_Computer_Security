/*
 * FAT32 BMP Recovery Tool
 * ------------------------------------------------------------
 * Purpose:
 *   Scan a raw FAT32 disk image, identify directory entries that
 *   reference .BMP files, and extract the corresponding file data
 *   into temporary files while printing their SHA‑1 checksums.
 *
 * High‑level algorithm
 * --------------------
 *   1. Map the entire image into memory (mmap) for random access.
 *   2. Derive basic layout parameters from the BIOS Parameter Block
 *      (sectors per cluster, first data sector, total clusters…).
 *   3. For every data cluster (cluster ≥ 2):
 *        • Examine the first 32 bytes.  If they look like a valid
 *          directory entry (short or long) the cluster is *possibly*
 *          a directory cluster → pass it to search_cluster().
 *   4. Inside search_cluster():
 *        • Walk 32‑byte steps until the end of the cluster, grouping
 *          consecutive LFN dirents plus the following SFN dirent into
 *          a single *file record*.
 *        • Records that are completely contained in the cluster are
 *          sent directly to handle().
 *        • Head fragments (LFN parts at the *end* of the cluster with
 *          no SFN yet) and tail fragments (LFN/SFN at the *start* of
 *          the cluster that belong to the previous cluster) are pushed
 *          into waiting.heads / waiting.tails for later matching.
 *   5. After the full sweep, match_entries() pairs every head with the
 *      correct tail via the FAT checksum field and feeds the combined
 *      record to handle().
 *   6. handle():
 *        • Reject dirents that are deleted, point to directories, or
 *          whose first data cluster does not start with the BMP magic
 *          bytes "BM".
 *        • Build the long file name by concatenating LFN pieces.
 *        • Copy <file_size> bytes starting from the first data cluster
 *          into a temporary file, compute SHA‑1, print hash + name.
 *
 * Design assumptions / limitations
 * --------------------------------
 *   • The FAT may be zeroed; the program does *not* follow cluster
 *     chains.  It assumes the file data are stored contiguously.
 *   • Cluster size is a multiple of 512 bytes; dirents are always
 *     cluster‑aligned, so testing the first dirent is a cheap filter.
 *   • Only BMP files are of interest; max file size limited to 64 MiB
 *     (lab requirement).
 *   • waiting_entries arrays are sized for teaching images; enlarge
 *     them for real‑world use.
 */

 #include <assert.h>
 #include <fcntl.h>
 #include <stdbool.h>
 #include <stdio.h>
 #include <stdlib.h>
 #include <string.h>
 #include <sys/mman.h>
 #include <unistd.h>
 #include "fat32.h"
 
 /* ------------------------------------------------------------
  * Debug helper
  * ----------------------------------------------------------*/
 static bool debug_enabled = true;
 #define DEBUG_PRINT(...) do { if (debug_enabled) printf(__VA_ARGS__); } while (0)
 
 #define BMP_SIGNATURE 0x4D42          /* "BM" */
 #define TEMP_FILE_TEMPLATE "/tmp/fsrecov_XXXXXX"
 
 /* LFN attribute mask as defined by Microsoft FAT spec */
 #define ATTR_LONG_NAME (ATTR_READ_ONLY | ATTR_HIDDEN | ATTR_SYSTEM | ATTR_VOLUME_ID)
 #define LAST_LONG_ENTRY 0x40          /* Bit 6 set in the *first* LFN dirent */
 
 /* ------------------------------------------------------------
  * Local structures (packed to match on‑disk layout)
  * ----------------------------------------------------------*/
 struct fat32lfn {
     u8  LDIR_Ord;          /* Sequence number (bit6 = LAST_LONG_ENTRY) */
     u16 LDIR_Name1[5];     /* First 5 UTF‑16 characters */
     u8  LDIR_Attr;         /* Must be ATTR_LONG_NAME */
     u8  LDIR_Type;         /* Must be 0 */
     u8  LDIR_Chksum;       /* Checksum of the corresponding SFN */
     u16 LDIR_Name2[6];     /* Next 6 UTF‑16 characters */
     u16 LDIR_FstClusLO;    /* Must be 0 for LFN */
     u16 LDIR_Name3[2];     /* Last 2 UTF‑16 characters */
 } __attribute__((packed));
 
 struct output_file {
     char *name;            /* Long file name (UTF‑8, null‑terminated) */
     u8   *start;           /* Pointer to first byte of file data */
     u32   size;            /* Bytes to copy */
 };
 
 /* Head/tail fragments for cross‑cluster LFN chains */
 struct entry_part {
     void *entry;           /* Pointer to first dirent of fragment */
     int   len;             /* Number of dirents in fragment */
 };
 
 struct waiting_entries {
     struct entry_part heads[10];
     struct entry_part tails[10];
     int head_count;
     int tail_count;
 };
 
 /* ------------------------------------------------------------
  * Global variables (derived at runtime)
  * ----------------------------------------------------------*/
 struct fat32hdr *hdr;      /* Pointer to boot sector (also mmap base) */
 u8 *disk_base;            /* Same as (u8*)hdr */
 u8 *disk_end;             /* Last valid byte in the image */
 int first_data_sector;     /* LBA of cluster 2 */
 int total_clusters;        /* #clusters in data region */
 const int entry_size = sizeof(struct fat32dent);
 static struct waiting_entries waiting = {0};
 
 /* ------------------------------------------------------------
  * Forward declarations
  * ----------------------------------------------------------*/
 void *mmap_disk(const char *);
 void full_scan(void);
 void search_cluster(u8 *cluster_start, int clus_num);
 void handle(u8 *entry_start, int len);
 void match_entries(void);
 
 u8* first_byte_ptr_of_cluster(int clus_num);
 bool is_dirent_cluster_possibly(u8* cluster_start);
 bool is_dirent_basic(struct fat32dent* dent);
 bool is_dirent_long(struct fat32lfn* lfn);
 void extract_name_from_lfn(struct fat32lfn* lfn, char* out);
 void outprint(struct output_file f);
 bool matched(struct fat32lfn* head, struct fat32lfn* tail);
 u8 calc_checksum(const u8* name);


 /* ------------------------------------------------------------
  * Program entry
  * ----------------------------------------------------------*/
 int main(int argc, char *argv[])
 {
     if (argc < 2) {
         fprintf(stderr, "Usage: %s <fat32‑image>\n", argv[0]);
         exit(EXIT_FAILURE);
     }
 
     setbuf(stdout, NULL); /* Unbuffered stdout for progress/debug */
 
     /* Sanity checks against struct padding mistakes */
     assert(sizeof(struct fat32hdr)  == 512);
     assert(sizeof(struct fat32dent) == 32);
 
     disk_base = mmap_disk(argv[1]);
     hdr       = (struct fat32hdr *)disk_base;
 
     disk_end  = disk_base + hdr->BPB_TotSec32 * hdr->BPB_BytsPerSec - 1;
     first_data_sector = hdr->BPB_RsvdSecCnt + hdr->BPB_NumFATs * hdr->BPB_FATSz32;
     total_clusters    = (hdr->BPB_TotSec32 - first_data_sector) / hdr->BPB_SecPerClus;
 
     full_scan();
 
     munmap(hdr, hdr->BPB_TotSec32 * hdr->BPB_BytsPerSec);
     return 0;
 }
 
 /* ------------------------------------------------------------
  * Memory‑map the entire image (read‑write, private)
  * ----------------------------------------------------------*/
 void *mmap_disk(const char *fname)
 {
     int fd = open(fname, O_RDWR);
     if (fd < 0) {
         perror("open");
         exit(EXIT_FAILURE);
     }
 
     off_t size = lseek(fd, 0, SEEK_END);
     if (size < 0) {
         perror("lseek");
         close(fd);
         exit(EXIT_FAILURE);
     }
 
     struct fat32hdr *h = mmap(NULL, size, PROT_READ | PROT_WRITE, MAP_PRIVATE, fd, 0);
     if (h == MAP_FAILED) {
         perror("mmap");
         close(fd);
         exit(EXIT_FAILURE);
     }
     close(fd);
 
     /* Minimal boot‑sector sanity */
     assert(h->Signature_word == 0xAA55);
     assert(h->BPB_TotSec32 * h->BPB_BytsPerSec == (u32)size);
 
     return h;
 }
 
 /* ------------------------------------------------------------
  * Sweep every data cluster and run the heuristic filter
  * ----------------------------------------------------------*/
 void full_scan(void)
 {
     for (int clus_num = 2; clus_num < total_clusters + 2; ++clus_num) {
         u8 *cluster_start = first_byte_ptr_of_cluster(clus_num);
         if (is_dirent_cluster_possibly(cluster_start)) {
             search_cluster(cluster_start, clus_num);
         }
     }
     /* second pass – join cross‑cluster fragments */
     match_entries();
 }
 
 /* Return pointer to the first byte of the given cluster */
 u8 *first_byte_ptr_of_cluster(int clus_num)
 {
     int first_sector_of_cluster = first_data_sector + (clus_num - 2) * hdr->BPB_SecPerClus;
     return disk_base + (size_t)first_sector_of_cluster * hdr->BPB_BytsPerSec;
 }
 
 /* ------------------------------------------------------------
  * Quick test: does the first dirent in this cluster *look like*
  * a valid LFN or SFN?  If not, we assume the cluster is pure data.
  * ----------------------------------------------------------*/
 bool is_dirent_cluster_possibly(u8 *cluster_start)
 {
     return is_dirent_basic((struct fat32dent *)cluster_start) ||
            is_dirent_long ((struct fat32lfn  *)cluster_start);
 }
 
 /* Validate a short (8.3) dirent – relaxed rules, good for heuristics */
 bool is_dirent_basic(struct fat32dent *dent)
 {
     /* 0x00 marks "no more entries" → definitely *not* a directory cluster */
     if (dent->DIR_Name[0] == 0x00)
         return false;
 
     /* Reserved bits must be zero */
     if ((dent->DIR_Attr & 0b11000000) != 0 || dent->DIR_NTRes != 0)
         return false;
 
     /* "..", deleted (0xE5) and "." are allowed – early accept */
     if (dent->DIR_Name[0] == '.' || dent->DIR_Name[0] == 0xE5)
         return true;
 
     /* Cluster range sanity */
     int clus = (dent->DIR_FstClusHI << 16) | dent->DIR_FstClusLO;
     if (clus < 2 || clus > total_clusters + 1)
         return false;
 
     /* Lab constraint: file size ≤ 64 MiB */
     if (dent->DIR_FileSize > 64 * 1024 * 1024)
         return false;
 
     return true;
 }
 
 /* Validate a Long File Name dirent */
 bool is_dirent_long(struct fat32lfn *lfn)
 {
     int ord = lfn->LDIR_Ord & ~LAST_LONG_ENTRY; /* strip bit6 */
     if (ord == 0 || ord > 20)               /* 13×20 = 260 UTF‑16 chars max */
         return false;
 
     if (lfn->LDIR_Attr != ATTR_LONG_NAME || lfn->LDIR_Type != 0)
         return false;
 
     if (lfn->LDIR_FstClusLO != 0)
         return false;
 
     return true;
 }
 
 /* ------------------------------------------------------------
  * Deep scan of a directory cluster – extract complete records
  * and cache head/tail fragments for the second pass.
  * ----------------------------------------------------------*/
 void search_cluster(u8 *cluster_start, int clus_num)
 {
     u8 *p = cluster_start;
 
     /* --------------- handle potential *tail* fragment --------------- */
     if (is_dirent_long((struct fat32lfn *)p) && !(((struct fat32lfn *)p)->LDIR_Ord & LAST_LONG_ENTRY))
    {
         /* LFN that is *not* the first (bit6=0) → must belong to prev. cluster */
         waiting.tails[waiting.tail_count++] = (struct entry_part){ p, ((struct fat32lfn *)p)->LDIR_Ord + 1 };
         p += entry_size * (((struct fat32lfn *)p)->LDIR_Ord + 1);
     } else if (is_dirent_basic((struct fat32dent *)p)) {
         /* Single SFN at cluster start – also a tail fragment */
         waiting.tails[waiting.tail_count++] = (struct entry_part){ p, 1 };
         p += entry_size;
     }
 
     DEBUG_PRINT("  Cluster %d: offset 0x%tx\n", clus_num, p - disk_base);
 
     /* --------------- main walk inside this cluster --------------- */
     u8 *curr = p;          /* first dirent of current record */
     int curr_entries = 0;  /* #LFN collected so far */
 
     while (p < cluster_start + hdr->BPB_BytsPerSec * hdr->BPB_SecPerClus) {
         if (is_dirent_basic((struct fat32dent *)p)) {
             /* Reached SFN → record complete */
             handle(curr, curr_entries + 1);
             p += entry_size;
             curr = p;
             curr_entries = 0;
         } else if (is_dirent_long((struct fat32lfn *)p)) {
             p += entry_size;
             ++curr_entries;
         } else {
             /* Something that does not look like a dirent → stop */
             DEBUG_PRINT("  break at offset 0x%tx\n", p - disk_base);
             break;
         }
     }
 
     /* --------------- possible *head* fragment --------------- */
     if (curr != p) {
         waiting.heads[waiting.head_count++] = (struct entry_part){ curr, curr_entries };
     }
 }
 
 /* ------------------------------------------------------------
  * Given a complete record (LFN×n + SFN), validate & extract BMP
  * ----------------------------------------------------------*/
 void handle(u8 *entry_start, int len)
 {
     struct fat32dent *sfn = (struct fat32dent *)(entry_start + (len - 1) * entry_size);
 
     /* Reject directories, deleted entries, or bogus cluster numbers */
     int clus = (sfn->DIR_FstClusHI << 16) | sfn->DIR_FstClusLO;
     if (clus == 0 || clus > total_clusters + 1 || (sfn->DIR_Attr & ATTR_DIRECTORY) || sfn->DIR_Name[0] == 0xE5)
         return;
 
     /* Verify BMP magic */
     if (*(u16 *)first_byte_ptr_of_cluster(clus) != BMP_SIGNATURE)
         return;
 
     /* Build long file name (UTF‑8, simple ANSI truncation) */
     char long_name[256] = {0};
     for (int i = len - 2; i >= 0; --i) {
         struct fat32lfn *lfn = (struct fat32lfn *)(entry_start + i * entry_size);
         char part[14];
         extract_name_from_lfn(lfn, part);
         strncat(long_name, part, sizeof(long_name) - strlen(long_name) - 1);
     }
     if (long_name[0] == '\0')
         strncpy(long_name, (char *)sfn->DIR_Name, 13);
 
     /* File size & bounds check */
     int file_size = sfn->DIR_FileSize;
     u8 *data_start = first_byte_ptr_of_cluster(clus);
     if (data_start + file_size > disk_end)
         file_size = disk_end - data_start + 1;
 
     struct output_file f = { long_name, data_start, (u32)file_size };
     outprint(f);
 }
 
 /* Extract 13 ANSI characters from one LFN dirent (lower byte of UTF‑16) */
 void extract_name_from_lfn(struct fat32lfn *lfn, char *out)
 {
     for (int i = 0; i < 5; ++i)  out[i]      = lfn->LDIR_Name1[i];
     for (int i = 0; i < 6; ++i)  out[i + 5]  = lfn->LDIR_Name2[i];
     for (int i = 0; i < 2; ++i)  out[i + 11] = lfn->LDIR_Name3[i];
     out[13] = '\0';
 }
 
 /* ------------------------------------------------------------
  * Write the recovered file to /tmp, compute SHA‑1 and print
  * ----------------------------------------------------------*/
 void outprint(struct output_file f)
 {
     char path[] = TEMP_FILE_TEMPLATE;
     int fd = mkstemp(path);
     assert(fd >= 0);
 
     write(fd, f.start, f.size);
     close(fd);
 
     char cmd[256];
     snprintf(cmd, sizeof(cmd), "sha1sum %s", path);
     FILE *fp = popen(cmd, "r");
     char sha1[64];
     fscanf(fp, "%63s", sha1);
     pclose(fp);
 
     printf("%s  %s\n", sha1, f.name);
     unlink(path);
 }
 
 /* ------------------------------------------------------------
  * Second pass – pair head & tail fragments using checksum
  * ----------------------------------------------------------*/
 void match_entries(void)
 {
     for (int i = 0; i < waiting.head_count; ++i) {
         u8 *head = waiting.heads[i].entry;
         int head_len = waiting.heads[i].len;
 
         for (int j = 0; j < waiting.tail_count; ++j) {
             if (!waiting.tails[j].entry)
                 continue;
 
             u8 *tail = waiting.tails[j].entry;
             int tail_len = waiting.tails[j].len;
 
             if (matched((struct fat32lfn *)head, (struct fat32lfn *)tail)) {
                 DEBUG_PRINT("Matched: head 0x%tx – tail 0x%tx\n", head - disk_base, tail - disk_base);
                 size_t bytes = (head_len + tail_len) * entry_size;
                 u8 *buf = malloc(bytes);
                 memcpy(buf, head, head_len * entry_size);
                 memcpy(buf + head_len * entry_size, tail, tail_len * entry_size);
                 handle(buf, head_len + tail_len);
                 free(buf);
                 waiting.tails[j].entry = NULL; /* consumed */
             }
         }
     }
 
     /* Any tail fragment that is a single SFN may represent a file with no LFN */
     for (int i = 0; i < waiting.tail_count; ++i) {
         if (waiting.tails[i].entry && waiting.tails[i].len == 1)
             handle(waiting.tails[i].entry, 1);
     }
 }
 
 /* Check whether two fragments belong to the same file (checksum match) */
 bool matched(struct fat32lfn *head, struct fat32lfn *tail)
 {
     assert(is_dirent_long(head));
 
     if (is_dirent_long(tail))
         return head->LDIR_Chksum == tail->LDIR_Chksum;
 
     if (is_dirent_basic((struct fat32dent *)tail))
         return head->LDIR_Chksum == calc_checksum(((struct fat32dent *)tail)->DIR_Name);
 
     return false; /* should not reach */
 }
 
 /* Standard FAT checksum algorithm for SFN (11 bytes) */
 u8 calc_checksum(const u8 *name)
 {
     u8 sum = 0;
     for (int i = 11; i; --i)
         sum = ((sum & 1) ? 0x80 : 0) + (sum >> 1) + *name++;
     return sum;
 }
 