# README: Understanding Privilege Dropping and File Descriptor Leakage  

## Overview  

This program demonstrates a **privilege leak** scenario where a process with elevated privileges opens a protected system file (`/etc/zzz`), drops its privileges, and then executes a shell.  

By carefully managing file descriptors, we can observe how the program retains access to the protected file even after dropping privileges, and how closing the file descriptor prevents further access.  

---

## Compilation and Setup  

### **1. Create and Configure `/etc/zzz`**  

Before running the program, create the protected system file and set its ownership and permissions:

```sh
sudo touch /etc/zzz              # Create the file
sudo chmod 666 /etc/zzz          # Temporarily make it writable
nano /etc/zzz                    # Edit the file and write some text
sudo chmod 644 /etc/zzz          # Restrict write access to root
sudo chown root:root /etc/zzz    # Ensure root owns the file
```

### **2. Compile the Program**
```sh
gcc cap_leak.c -o cap_leak
```

### **3. Set the Executable to Run with Root Privileges (Set-UID)**
```sh
sudo chown root cap_leak
sudo chmod 4755 cap_leak
ls -l cap_leak   # Verify it has Set-UID bit set (-rwsr-xr-x)
```
This allows `cap_leak` to run with root privileges when executed by a regular user.

---

## Running and Observing the Privilege Leak  

### **1. Run `cap_leak` and Observe Behavior**
```sh
./cap_leak
```
Expected output:
```
fd is 3
```
This confirms that `/etc/zzz` was opened with file descriptor `3`.  

After execution, a new shell (`/bin/sh`) is spawned, but the program has dropped its privileges (`setuid(getuid())`).  

### **2. Check User ID in the New Shell**
Run:
```sh
id
```
Expected output:
```
uid=1000(user) gid=1000(user) groups=1000(user)
```
This confirms that the program has dropped privileges and is now running as a normal user.

### **3. Check File Write Access**
In the new shell, try writing to `/etc/zzz`:
```sh
echo "test" >> /etc/zzz
```
If successful, this indicates a privilege leak. The file was opened before dropping privileges, and the descriptor remains valid.

---

## Closing the File Descriptor to Prevent Leakage  

### **1. Close File Descriptor 3**
Before executing `cap_leak`, run:
```sh
exec 3>&-
```
This explicitly closes file descriptor `3`.

### **2. Run `cap_leak` Again**
```sh
./cap_leak
```
Now, in the new shell, attempt to write to `/etc/zzz`:
```sh
echo "test" >> /etc/zzz
```
Expected result:
```
Permission denied
```
Since the file descriptor was closed before executing the program, it can no longer be used to write to `/etc/zzz`.

---

## Explanation of the Behavior  

1. **Why does `cap_leak` allow writing to `/etc/zzz` after privilege drop?**  
   - The file is opened *before* dropping privileges, meaning the file descriptor remains valid even after `setuid(getuid())`.
   - Since Linux does not revoke open file descriptors when privileges are dropped, the process can continue using the file descriptor.

2. **How does `exec 3>&-` prevent the leak?**  
   - `exec 3>&-` explicitly closes file descriptor `3`, preventing the shell from inheriting it.
   - Without this open file descriptor, the process no longer has access to `/etc/zzz`.

---

## Key Takeaways  

- **File descriptors remain valid after privilege drops**, leading to potential security risks.  
- **Set-UID programs should be carefully designed** to close unnecessary file descriptors before executing lower-privileged processes.  
- **Explicitly closing file descriptors** before running a privileged program can prevent unintended access.  

