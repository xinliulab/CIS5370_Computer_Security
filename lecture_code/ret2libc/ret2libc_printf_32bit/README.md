# Return-to-libc Attack Research Guide

This document provides an educational overview of buffer overflow vulnerabilities and return-to-libc attacks for security researchers and students. This knowledge is intended for:

- Security researchers conducting authorized testing
- Students learning about system security
- Developers working to prevent these vulnerabilities

## ⚠️ Important Notice

This research must only be conducted in controlled lab environments with proper authorization. Unauthorized testing or exploitation of vulnerabilities is illegal and unethical.

## Prerequisites

- Linux development environment
- GCC compiler
- Basic understanding of C programming
- Knowledge of assembly and memory layout

## Research Environment Setup

1. Configure development environment for controlled testing:

```bash
# Install required development tools
sudo apt-get install gcc-multilib
```

2. Compile test program with security flags for research:

```bash
gcc -m32 -fno-stack-protector -o stack stack.c
```

## Analysis Process

### Setp 1. Memory Layout Analysis

Using GDB to understand program memory structure:

```bash
gdb ./stack
```

### Step 2. Function Address Resolution

Inside `gdb`, run:

```bash
(gdb) b foo
(gdb) r
```

### Step 3. Function Address Resolution

Then check the address of `printf()`, `exit()`, and `$ebp` (frame pointer):

```bash
(gdb) p printf
(gdb) p exit
(gdb) p/x $ebp
```

### Step 4. Function Address Resolution

You can also locate the address of `"/bin/sh"` in `libc`:

```bash
(gdb) find 0xf7e00000, 0xf7ffffff, "/bin/sh"
```

## Step 5: Finding Instruction Addresses

In gdb, disassemble the target function:

```bash
(gdb) disassemble foo
```

Locate the `leave; ret` sequence:

```nasm
0x08048565 <+XX>: leave
```

### Step 6: Execute `stack`

Run:

```bash
./stack
```
