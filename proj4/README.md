# 1. Background

Today, we tend to back up important files to cloud storage or local storage devices. While this increases data reliability, what if you accidentally quick-format your SD card (nowadays usually formatted as a FAT file system for compatibility)? Even if the file system directory structures are destroyed, the data may still physically exist — and in that case, we can still recover it!

To recover such data, we need to understand how formatting works. Note that a file system is essentially a data structure built on a disk. If you care about data structures, consider the following analogy:

```c
root->left = root->right = NULL;
```

The rest of the data structure is effectively lost — the data structure has completed a one-time "memory cleanup". Once this overall structure is discarded, it is possible to reallocate memory blocks for new structures, so the data previously stored on disk may get overwritten or newly formatted.

This explains why a quick format on a 1TB disk can complete so quickly — it only resets the file system metadata. Fortunately, file systems offer various recovery options. More importantly, in many cases (especially in quick formats), data structures are not fully erased.

# 2. Project Description

## 2.1 Project Goal: Recover Image Data from a Quick-Formatted FAT32 File System

Using the `fsrecov` tool, given a FAT32 file system that has been quick-formatted using `mkfs.vfat`, attempt to recover image files (especially BMP images) that existed prior to formatting. You are expected to recover visually intact image files from the disk.

---

# 3. Reference Image

We provide a sample file system image for reference. The actual test images are from the same dataset (WikiArt), but we may select different images and randomly rename or resize them. Nevertheless, all images maintain the same characteristics (e.g., similar file name length distribution). You can [download the reference image here](#).

Once downloaded, you can directly mount the image file as a file system (you may need `root` privileges). The image will appear as part of the file system:

```bash
$ mount /tmp/fsrecov.img /mnt/
$ tree /mnt/
/mnt/
└── DCIM
    ├── 0M15CwG1yP32UCPc.bmp
    ├── 1fy0sw8n6.bmp
    └── 2Kbg82NaSqPga.bmp
```

You can view any of the image files, for example, `3DhTVVP9avTrH.bmp` corresponds to the following image:

![Alt text](3DhTVVP9avTrH.bmp)

---

# 4. Inspecting the File System with Binary Tools

If you use a binary inspection tool (such as `xxd`) to view the disk image, you’ll find that the FAT table is intact. The FAT table uses a linked list structure to store the next data block (in FAT, this means the next *cluster*) for each image file:

```
00004000: f8ff ffff 0fff ffff 818f 1720 0000 .... ...........
00004010: 0500 0000 0600 0000 0700 0000 0800 0000 ............
00004020: 0900 0000 0a00 0000 0b00 0000 0c00 0000 ............
00004030: 0d00 0e00 0e00 0000 0f00 0000 1000 0000 ............
00004040: 1100 0000 1200 0000 1300 0000 1400 0000 ............
```

---

Next, you can simulate the operation performed by the Online Judge when testing your recovery code: use `mkfs.fat` to **quick format** the disk image:

```bash
$ mkfs.fat -v -F 32 -S 512 -s 8 fsrecov.img
mkfs.fat 4.1 (2017-01-24)
WARNING: Not enough clusters for a 32 bit FAT!
/tmp/fsrecov.img has 64 heads and 32 sectors per track,
hidden sectors 0x0000;
logical sector size is 512,
using 0xf8 media descriptor, with 131072 sectors;
drive number 0x80;
filesystem has 2 32-bit FATs and 8 sectors per cluster.
FAT size is 128 sectors, and provides 16348 clusters.
There are 32 reserved sectors.
Volume ID is a3320dad, no volume label.
```

---

If you mount the image again after this quick format, you’ll see an **empty directory**, meaning that all file metadata on the disk image has been deleted:

```bash
$ tree /mnt/
/mnt/
└── (empty)

0 directories, 0 files
```

---

# 5. Verifying Data Loss After Quick Format

If you **mount the image again after the quick format**, you will see an empty directory. This confirms that all file entries on the disk image have been removed:

```bash
$ tree /mnt/
/mnt/
└── (empty)

0 directories, 0 files
```

---

# 6. Inspecting the FAT Table Post-Format

If you inspect `fsrecov.img` again using a binary tool like `xxd`, you’ll see that the FAT entries have been “cleared”:

```
00004000: f8ff ffff 0fff ffff 0000 0000 0000 0000  ................
00004010: 0000 0000 0000 0000 0000 0000 0000 0000  ................
...
```

---

# 7. Traces Still Exist

Although the file system no longer recognizes files on the disk, **you can still find traces** if you dig deeper using a tool like `grep`. Some “breadcrumbs” remain:

```
00025ae0: 4250 0043 0070 002e 0062 006d 0080 6d00  B.P..C.p...b.m..m.
00025af0: 7000 0062 ffff ffff 0000 0000 0000 0000  p..b..............
...
00025b40: 0100 0301 0031 0043 0035 0047 0031 0059  .....1.C.5.G.1.Y.
00025b50: 0050 0033 0052 0055 0050 0043 002e 0062  .P.3.R.U.P.C...b.
00025b60: 006d 0070 4d36 3135 4357 7e31 4c2d 5020  .m.p.M615CW~1L-P 
00025b70: 0020 5050 2e7a 5020 6915 3677 0700 0700   .PP.zP.i.6w....
```

It appears that some **directory entry format** (FAT32-style) still stores metadata related to `0M15CwG1yP32UCPc.bmp`. Additionally, the file’s BMP header and content are still partially present in the data region:

```
0002a4d0: 424d 2ecf 0f00 0000 3600 0000 2800 0000  BM......6...(...
0002a4e0: 8000 0000 6400 0000 0100 1800 0000 0000  ....d...........
...
0002a500: 0000 0000 0000 0000 0000 0000 0000 0000  ................
...
0002b000: 7e4d 8076 984d 7ca2 766d 9469 6187 6a64  ~M.v.M|.vm.i.a.jd
```

---

# 8. Expected Output Example

If your `fsrecov` tool successfully recovers this image after formatting, your output might include something like:

```
d60e7d3db2d7419148af5b0ba524068b6ec6ef83  0M15CwG1yP32UCPc.bmp
```


---

# 9. Verifying Recovery with SHA1 Checksums

If you mount the **original (unformatted)** `fsrecov.img`, you can compute the SHA1 checksums of all BMP images to verify which ones you have successfully recovered.

```bash
$ cd /mnt/DCIM && sha1sum *.bmp
d60e7d3d2b47d19418af5b0ba52406b86ec6ef83  0M15CwG1yP32UPCp.bmp
1ab8c4f2e61903ae2a00d0820ea0111fac04d9d3  1yh0sw8n6.bmp
1681e23d7b8bb0b36c399c065514bc04badfde79  2Kbg82NaSqPga.bmp
aabd1ef8a2371dd64fb64fc7f10a0a31047d1023  2pxHTrpI.bmp
```

You can use these checksums to compare against the output of your recovery tool to determine which files were correctly recovered.

--- 

以下是你这段内容翻译成英文 `README.md` 格式：

---

# 10 FAT32 File System

We provide a **local copy of the Microsoft FAT Specification** for your reference. Additionally, the starter code includes an example of how to traverse the file system. However, we strongly **recommend not copying the reference code directly**, but instead writing your own logic — this will give you better training and understanding.

The FAT file system consists of:
- Some header metadata,
- A **FAT (File Allocation Table)**,
- And a **data region** (also known as "clusters", the official Microsoft translation being “簇”).

Through analysis, you’ll find:

- The **FAT header is still intact** — because we performed the format operation in the same way both times;
- The **FAT table has been cleared**, meaning this part of the data is entirely lost;
- The **data region is mostly unchanged** — because we used a *quick format*.

Now, you can begin referring to the specification and implement FAT parsing manually. For instance, the FAT32 specification defines the layout of the first 512 bytes of the file system in great detail. We also provide a corresponding structure in `fat32.h`:

```c
struct fat32hdr {
    u8  BS_jmpBoot[3];
    u8  BS_OEMName[8];
    u16 BPB_BytsPerSec;
    u8  BPB_SecPerClus;
    ...
    u8  __padding_1[420];
    u16 Signature_word;
} __attribute__((packed));
```

You are encouraged to conduct your own experiments — using the provided disk image or one you create yourself — to try locating useful data structures within the file system.

Although a real file system is stored on disk (a block device), once you `mmap` it, it becomes an in-memory data structure. You can access and manipulate it directly, though you’ll need to **manually compute some pointer offsets** to access the correct memory locations.

---

以下是这段内容翻译为英文 `README.md` 格式的版本：

---

# 11. BMP File Format

To complete this project, you’ll also need to understand the **BMP file format**. We expect you to independently study and learn this format.

The good news is that the version of BMP files we use in this project is consistent, so you can safely ignore most of the fields in the BMP header. Instead, you only need to focus on the following:

- The **offset** to the beginning of the bitmap data region;
- The **width and height** of the image.

There are plenty of helpful resources online to assist you in understanding the BMP format. You may refer to:
- [Wikipedia - BMP File Format](https://en.wikipedia.org/wiki/BMP_file_format)
- [Microsoft Bitmap (BMP) File Structure](https://learn.microsoft.com/en-us/windows/win32/gdi/bitmap-storage)

---

以下是你提供的内容翻译为英文的 `README.md` 形式：

---

# 12.  Image Data Recovery

Next, you’ll need to **recover directory entries** from the file system, which will allow you to obtain the **file names** of the images. The FAT manual provides a precise specification for directory entries. If you're unsure how to decode them, there are many blog posts and tutorials available to help.

> ⚠️ Be careful with entries that may span multiple clusters. For now, we’ll assume all directory entries are stored within a single cluster, which simplifies things. You can skip recovery of multi-cluster directories and your program won't crash if you handle this properly.

The figure below shows an example of a short file name entry followed by a long file name (LFN) stored across multiple entries:

![Directory Entry Format](directory_entry_example.png) <!-- Add actual image path -->

Once you recover a valid directory entry, you obtain not only the **correct file name**, but also **useful metadata** such as the **starting cluster ID**. With this cluster ID, you can locate the corresponding data block and determine whether it contains a BMP header.

- If it does, you can treat it as a valid image file.
- If the cluster does not contain a BMP header, you may safely discard that file during recovery.

---

# 13. Recovering the Bitmap Data

Once you’ve identified the starting cluster of the image, the next part is easier. In the simplest case, **assume the image is stored in continuous clusters**. Using the file size (which is also in the directory entry), you can determine exactly how many clusters to read and recover the full BMP.

If you want to go further, you’ll need to parse the FAT table and perform cluster “chaining” analysis. For example, you can try solving the following challenges:

- Reconstruct a linked list of clusters based on the FAT table;
- Handle files that span non-contiguous clusters;
- Use cluster chaining to determine how images are fragmented on disk;
- Rebuild BMP images from multiple non-sequential clusters.

---

# 14. Extra Challenge: Understanding Images

What if you want to **go beyond just recovery** and think about what the image actually represents?

One basic way to think about images is as a **function of space and time**. For example, you can treat an image as a function \( f(x, y, t) \), where \( x, y \) are spatial coordinates and \( t \) is time (for animations or video). This leads to partial derivatives such as:

\[
\frac{\partial z}{\partial t},\quad \frac{\partial z}{\partial x},\quad \frac{\partial z}{\partial y}
\]

These are useful in image processing tasks such as motion detection, edge detection, and more.

---


以下是你这段内容翻译为英文 `README.md` 的版本：

---

# 15. Checksum Verification

At this point, you’ve recovered the **file name**, **file size**, and **file content** — it's time to compute the **checksum**.

In this experiment, you are required to invoke the external tool `sha1sum` to calculate the checksum. `sha1sum` reads data from a file or from standard input (`stdin`) and prints the hash digest to standard output.

Example:

```bash
$ echo "Hello, World" | sha1sum
4ab299c8ad6ed14f31923dd94f8b5f5cb89dfb54  -
```

### What is a checksum?

A checksum (also known as a fingerprint) is the result of a **one-way hash function** `H(x)` that maps a large string space into a small, fixed-length output. The key property is that it's **computationally infeasible** to reverse-engineer the original input `x` from a given hash value `t` such that `H(x) = t`.

Checksums help quickly verify file equality without comparing entire contents. They are also widely used for **data deduplication**. For instance, in many messaging apps (like QQ or WeChat), when you upload a file, the server first checks the checksum — if someone has already uploaded the same file, it instantly completes the transfer, leading to the phenomenon known as “秒传” (instant transmission).

---

### How to Call `sha1sum` from C

If you don’t want to manually manage `pipe-fork-execve` calls in your C code, `popen()` is a simpler alternative.

You can write the recovered image to a temporary file, and then use `popen()` to invoke `sha1sum` and capture its output:

```c
FILE *fp = popen("sha1sum /tmp/your-tmp-filename", "r");
panic_on(fp < 0, "popen");
fscanf(fp, "%s", buf); // Get it!
pclose(fp);
```

---

# 16. Debugging Your Program

This project involves a relatively long program. It’s recommended to add **logging** to help track your program’s behavior. However, be cautious not to interfere with the **Online Judge’s output format**.

Keep in mind:
- You are limited to a **single `.c` file** for submission.
- Maintain clean code style and use an IDE with **code folding** to boost your productivity.

---

### Why Didn't It Work?

A common frustration is when your program doesn’t recover the image correctly — even though you compute the `sha1sum`, the result looks like a random number. Remember: a single byte of difference will cause the entire hash to change.

### 🧪 Pro Tip: Visual Debugging

A great way to debug is through **visual inspection**:
- Output recovered images to a temporary folder.
- Open the images and **inspect them manually**.

You’ll find that some images are fully recovered while others are corrupted. Can you explain **why** some of the images look like this?

> (💡 Insert corrupted vs. correct BMP example here if applicable.)

---

