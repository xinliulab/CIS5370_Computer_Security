# 🧪 64-Bit Format String Exploit Lab Guide

This guide walks you through exploiting a **64-bit format string vulnerability** to overwrite a return address and jump to custom shellcode.

We break the process into 3 main steps:

---

## ✅ Step 1: Calculate the Format String Offset

Before we can write to a target address using `%n` or `%hn`, we need to determine **which argument index** corresponds to our injected data on the stack.

### 1. Run the Format String Offset Finder

```bash
make clean
python3 format_offset_finder.py
```

This writes a payload of 60 `%p` format specifiers into `badfile`.

### 2. Run the Vulnerable Program

```bash
make
make run
```

You'll get an output similar to the following:

```
0x55555555a490 (nil) (nil) 0x78 (nil) 0x1 0x7fffffffdb20 0x7fffffffdb08 0x7fffffffdb00 0x7fffffffdbf0 0x55555555529a 0x7fffffffdd18 0x100000040 0x2520702520702520 ...
```

### 🧠 Why do you see `0x2520702520702520`?

This value represents the ASCII encoding of part of your format string:

```
0x25 = '%'
0x20 = ' '
0x70 = 'p'
```

So:
```
"% p% p% "  →  0x2520702520702520 (little-endian)
```

This confirms that your format string has landed on the stack.

---

### ✅ Determine the Argument Index

If `0x2520702520702520` appears at the **14th position**, then your format string starts at argument 14.

When building an exploit to overwrite the **return address**, which lies at an offset of 6 positions from the start of your format string, use:

```python
start_param = 14 + 6 = 20
```

This becomes the base argument index for your `%N$hn` writes.

---

## ✅ Step 2: Get Key Addresses

You need two important addresses:

1. The address of your input buffer (`str`) that contains the shellcode and format string
2. The base pointer (`rbp`) to calculate the location of the return address (`rbp + 8`)

### Instructions:

```bash
python3 fmtexploit_revised.py
gdb ./fmtvul
```

Inside GDB:

```gdb
break fmtstr
run
```

Once it hits the breakpoint:

```gdb
p $rbp     # Shows the base pointer
p str      # Shows the buffer address
```

For example:

```python
address_rbp =  0x7fffffffda60
address_str =  0x7fffffffda80
```

From this you can derive:

```python
address_ret = address_rbp + 8
address_binsh = address_str + 48 + 32  # 48: value, 32: address
```

## ✅ Step 3: Check

```gdb
gdb ./fmtvul
run
continue
```

## ✅ Step 4: Debug Safely Before Overwriting Return Address

### ⚠️ Why not write to the return address immediately?

If you overwrite the return address incorrectly, the program will `segfault` and you'll lose visibility into what happened.

### ✅ Safer approach:

Start by writing to a **non-critical address** in the stack, such as `0x7fffffffdb00`, which is part of your buffer (NOPs or unused space), and not:

- the return address
- or the shellcode region

### Instructions:

```gdb
x/40gx $rbp
```

Find a “safe” nearby address to test your `%N$hn` writes.

Once you confirm the format string correctly writes your target value to that test address, you can safely switch back to writing the real return address.

---