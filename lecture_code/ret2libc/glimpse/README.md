# Understanding ELF and Dynamic Linking

This guide demonstrates ELF structure and dynamic linking using a simple C program.

## Sample Code

```c
#include <stdlib.h>

int main()
{
    exit(0);
}
```

## Analysis Steps

### 1. Compile with Debug Symbols

```bash
gcc -g a.c
```

### 2. Examine the Binary

Check the file type:
```bash
file a.out
```

View ELF details:
```bash
readelf -a a.out
```

Key observation from relocation section:
```
Relocation section '.rela.plt' at offset 0x610 contains 1 entry:
  Offset          Info           Type           Sym. Value    Sym. Name + Addend
000000003fd0  000400000007 R_X86_64_JUMP_SLO 0000000000000000 exit@GLIBC_2.2.5 + 0
```

### 3. Dynamic Analysis with GDB

Start debugging:
```bash
gdb ./a.out
layout src
start
info inferiors
```

### 4. Memory Layout Analysis

Open a new terminal to view process memory map:
```bash
pmap <process_id>
```

### 5. Address Calculation

calculate actual memory address in Python:
```python
python3
hex(0x000000003fd0 + 0x0000555555554000)
```

### 6. Memory Watching in GDB

Set memory watchpoints:
```bash
watch *0x555555557fd0    # Watch for writes
rwatch *0x555555557fd0   # Watch for reads
```

Run to the watch point: 
```bash
c
```

Examine memory content:
```bash
p/x *0x555555557fd0
```

The p/x *0x555555557fd0 command prints the hexadecimal value stored at memory address 0x555555557fd0. In this context, that address corresponds to a Global Offset Table (GOT) entry pointing to the actual exit function in libc. Consequently, the output (for example, 0xf7c47ba0) is the runtime address in libc for exit (internally __GI_exit).

## Key Learning Points

1. **ELF Structure**
   - Debug symbols and their role
   - Relocation section contents
   - Symbol resolution mechanism

2. **Dynamic Linking**
   - PLT (Procedure Linkage Table) entries
   - GOT (Global Offset Table) usage
   - Runtime symbol resolution

3. **Memory Layout**
   - Process address space organization
   - Base address and offsets
   - Memory mapping of shared libraries

4. **Debug Tools**
   - Using GDB for runtime analysis
   - Memory watchpoints
   - Process memory mapping
