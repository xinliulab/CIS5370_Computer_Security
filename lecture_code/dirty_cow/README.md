# The Dirty COW Race Condition Attack

## mmap_example.c

'''
echo "12345678901234567890" > zzz
gcc mmap_example.c
./a.out
cat zzz
'''

## cow_map_readonly_file.c

'''
echo "12345678901234567890" > zzz
gcc cow_map_readonly_file.c
./a.out
'''

## cow_attack_passwd.c

Your explanation is clear and helpful! Here's a slightly polished version of your message to make it more readable and instructional, especially if you're sharing it with students or including it in lab instructions:

---

### 🐄 Dirty COW Exploit Demonstration – Kernel Check & Setup

To check your current Linux kernel version, run:

```bash
uname -r
```

> ⚠️ **Note:** Dirty COW no longer works on modern Linux systems because the kernel has been patched.  
> To successfully demonstrate the exploit, you must use an **old, unpatched kernel (before 2016)** in a **controlled or virtual environment**.

---

### ✅ If your kernel is old enough (pre-2016), you can try the following steps:

```bash
sudo useradd -u 1001 testcow
grep testcow /etc/passwd
gcc -Wall -pthread cow_attack_passwd.c
./a.out
```

Then, in another terminal, check if the modification worked:

```bash
grep testcow /etc/passwd
```

If you see the UID changed from `1001` to `0000`, the attack succeeded. 🐮✅

---

Let me know if you'd like a version that auto-checks the kernel or prints a warning if the exploit is likely to fail.
