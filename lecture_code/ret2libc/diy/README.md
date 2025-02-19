# dlbox: A Minimal Dynamic Linker and Loader

`dlbox` is a custom dynamic linker and loader designed to help students understand the fundamentals of dynamic linking. Unlike standard dynamic linking mechanisms (e.g., `ld.so` in Linux), `dlbox` provides a minimal and transparent way to load, link, and execute dynamically linked symbols through a custom symbol lookup table.

By using `dlbox`, students can observe how shared libraries are loaded, how symbols are resolved, and how execution is transferred to dynamically loaded code.

## How It Works

- **Custom Dynamic Library Format (`.dl`)**: Instead of using traditional `.so` files, `dlbox` works with custom `.dl` files, which contain both code and symbol information.
- **Custom Symbol Resolution**: Symbols are manually registered, imported, and exported in a lookup table.
- **Direct Execution of Dynamically Loaded Code**: `dlbox` allows executing functions stored in `.dl` files without relying on the OS loader.

## Usage

### 1. Build the Required Files

Generate `.dl` files from assembly sources:
```bash
make
```

This will compile and generate:
- `main.dl`
- `libhello.dl`
- `libc.dl`

### 2. Running and Exploring Dynamic Linking

Run a dynamically loaded executable:
```bash
./dlbox interp main.dl
```

#### Use main.s as an example to observe the dynamic linking process:

Inspect the preprocessed assembly file:
```bash
gcc -E main.S
```

Compile an object file with position-independent code:
```bash
gcc -m64 -fPIC -c main.S
```

Read symbol table from a dynamic library:
```bash
./dlbox readdl main.dl
```

The reason dlbox can run is that its .dl code contains only the .text section and does not use .data or .bss.

The .data and .bss sections are typically used for global variables in C, but dlbox primarily executes handwritten assembly code, where all variables are stored in registers and the stack, allowing it to bypass .data and .bss.

Disassemble an object file:
```bash
objdump -d main.o
```

View exported symbols from `libhello.dl`:
```bash
./dlbox readdl libhello.dl
```

# Understanding the Code: dlbox Implementation Details 

## 1. Custom Format Execution

`dlbox` uses a custom format that's still CPU-executable, based on 32-byte alignment. The format includes:

- **DL_HEAD**: File header containing
  - Magic number
  - File size
  - Code offset metadata

- **DL_CODE**: Section containing executable machine code

- **RECORD**: Symbol table for storing:
  - Import symbols (`?`)
  - Export symbols (`#`)
  - Load symbols (`+`)

Although this format differs from the standard ELF format, the `.dl` files still contain valid machine code that can be executed in memory.

## 2. Machine Code Generation

`dlbox` relies on `gcc` for assembly compilation but only uses it to generate `.o` files and manually extracts the `.text` section:

```c
void dl_gcc(const char *path) {
    char buf[256], *dot = strrchr(path, '.');
    if (dot) {
        *dot = '\0';
        sprintf(buf, "gcc -m64 -fPIC -c %s.S && "
                    "objcopy -S -j .text -O binary %s.o %s.dl",
                    path, path, path);
        system(buf);
    }
}
```

### Compilation Process

1. **GCC Compilation** (`gcc -m64 -fPIC -c`):
   - `-m64`: Generates 64-bit code
   - `-fPIC`: Generates Position-Independent Code for dynamic linking
   - `-c`: Generates only `.o` files without linking

2. **Object File Processing** (`objcopy`):
   - `-S`: Strips symbol information
   - `-j .text`: Extracts only the `.text` section
   - `-O binary`: Converts to pure binary format

The resulting `.dl` file contains only machine code, without ELF headers or symbol tables.

## 3. Custom Dynamic Loading

While `ld.so` handles ELF dynamic loading, `dlbox` implements its own loader:

```c
static struct dlib *dlopen(const char *path) {
    struct dl_hdr hdr;
    struct dlib *h;
    int fd = open(path, O_RDONLY);
    if (fd < 0) goto bad;
    if (read(fd, &hdr, sizeof(hdr)) < sizeof(hdr)) goto bad;
    if (strncmp(hdr.magic, DL_MAGIC, strlen(DL_MAGIC)) != 0) goto bad;

    h = mmap(NULL, hdr.file_sz, PROT_READ | PROT_WRITE | PROT_EXEC,
             MAP_PRIVATE, fd, 0);
    if (h == MAP_FAILED) goto bad;

    h->symtab = (struct symbol *)((char *)h + REC_SZ);
    h->path = path;

    for (struct symbol *sym = h->symtab; sym->type; sym++) {
        switch (sym->type) {
            case '+': dlload(sym); break;
            case '?': sym->offset = (uintptr_t)dlsym(sym->name); break;
            case '#': dlexport(sym->name, (char *)h + sym->offset); break;
        }
    }
    return h;

bad:
    if (fd > 0) close(fd);
    return NULL;
}
```

### Loading Process

1. Reads `.dl` file header and verifies magic number
2. Uses `mmap` to load the file into memory with execute permissions
3. Processes the symbol table:
   - `+`: Recursively loads dependent libraries
   - `?`: Resolves external symbols
   - `#`: Exports symbols for other `.dl` code

## 4. Working Mechanism

dlbox functions because:

1. **Machine Code Integrity**: 
   - The `.dl` files contain executable machine code generated by `gcc`
   - CPU can execute the code regardless of the file format

2. **Direct Code Loading**:
   - Uses `mmap` to load code directly
   - Manages function calls manually instead of using ELF loading

3. **Custom Dynamic Linking**:
   - Parses `.dl` symbol tables
   - Manually resolves symbols similar to ELF relocation tables

4. **Symbol Resolution**:
   - Implements custom `dlsym()` functionality
   - Replaces ELF's Global Offset Table (GOT) mechanism

## 5. Comparison with ELF Dynamic Linking

| Feature | ELF (`ld.so`) | dlbox (Custom) |
|---------|---------------|----------------|
| File Format | `.so` | `.dl` |
| Loading Method | `ld.so` loader | Manual `mmap()` |
| Symbol Resolution | ELF symbol table | Custom symbol table |
| Code Storage | ELF `.text` section | Raw `.text` binary |
| Dependency Resolution | ELF shared library | `dlload()` recursive |
| Symbol Lookup | GOT/PLT tables | Custom `dlsym()` |

## 6. Summary

- dlbox avoids ELF format in favor of using `gcc` for machine code generation and manual dynamic linking
- Execution is possible through direct `mmap` binary code mapping
- The system works because:
  - `.dl` files contain valid machine code
  - Dynamic linking is handled manually through custom symbol resolution
  - The custom loader provides necessary runtime support

dlbox serves as a lightweight dynamic loader that bypasses ELF complexity while maintaining functionality, making it an excellent educational tool for understanding dynamic linking concepts.


# Understanding Symbol Resolution in dlbox

## 1. What is `offset` in `dlsym()` and `dlexport()`?

- `syms[i].offset` stores the symbol (function or variable) offset within the `.dl` file, relative to the file's starting address
- During runtime, `.dl` files are not loaded using the ELF method, but rather mapped to dynamically allocated memory addresses using `mmap()`

### Example
Consider `func_hello` in `libhello.dl` located at offset `0x50` in the `.text` section:

```c
syms[i].name = "func_hello";
syms[i].offset = 0x50;  // Offset relative to .dl file
```

If `libhello.dl` is mapped to memory address `0x7fff1000` via `mmap()`:
- The actual function address = `0x7fff1000 + 0x50`

## 2. How does dlbox Correct `offset` for Proper Resolution?

dlbox uses a two-step resolution mechanism:

### Step 1: Symbol Export
In `dlexport()`, `offset` stores the global address:

```c
syms[i].offset = (uintptr_t)addr;  // addr is the global address after loading
```
- Here, `addr` is the global address after `mmap()` mapping, not the local offset within the `.dl` file

### Step 2: Symbol Resolution
In `dlsym()`, the returned `offset` is the global address:

```c
return (void *)syms[i].offset;
```
- When `main` looks up a symbol using `dlsym("func_hello")`, it directly receives the global address `0x7fff1000 + 0x50`, not the internal `.dl` offset

## Code Example

```c
// 1. dlexport() records symbol information
dlexport("func_hello", (void *)(mmap_base + sym->offset));

// 2. dlsym() resolves symbol, returns absolute address
void *addr = dlsym("func_hello");
```

This mechanism ensures that `main` can correctly resolve function addresses during runtime.

## Resolution Process Flow

1. Initial Loading:
   - `.dl` file is loaded into memory via `mmap()`
   - Base address is assigned (e.g., `0x7fff1000`)

2. Symbol Export:
   - Local offsets are converted to global addresses
   - Symbol table is updated with absolute addresses

3. Symbol Resolution:
   - `dlsym()` looks up symbols in the table
   - Returns ready-to-use function pointers

This approach simplifies symbol resolution by eliminating the need for runtime address calculation, as all addresses are pre-computed during the loading phase.

### **Observing Symbol Table Updates and Address Resolution in GDB**  

To analyze how `dlbox` updates the symbol table and resolves addresses, you can use **GDB** to set breakpoints at key locations in `dlbox.c` where symbols are processed.  

#### **1. Start GDB and Load dlbox**
```bash
gdb ./dlbox
```

#### **2. Set Breakpoints at Key Symbol Resolution Points**
```gdb
b dlbox.c:184  # Breakpoint at the start of the symbol resolution loop
b dlbox.c:191  # Breakpoint when a symbol is updated in the table
b dlbox.c:201  # Breakpoint when an address is returned to the caller
```

#### **3. Run dlbox with Dynamic Linking**
```gdb
run interp main.dl
```

#### **4. Observe the Symbol Table Updates**
When execution stops at each breakpoint, use the following commands to inspect how the symbols are being processed:

```gdb
print syms[i]      # Print the symbol entry being processed
print syms[i].name # Print the symbol name
print syms[i].offset # Check if the offset has been updated
print (void *)syms[i].offset # View the resolved function address
```

#### **5. Analyze Memory and Address Resolution**
```gdb
info registers    # Check register values, including the return address
x/10xg syms       # Examine the symbol table in memory
x/10i $rip        # View the next instructions to be executed
```

#### **6. Step Through Execution**
Use **`next` (n)** or **`step` (s)** to move through the symbol resolution process:
```gdb
n    # Execute the next line
s    # Step into function calls if necessary
```

By following these steps, you can **observe how symbols are added, resolved, and returned**, gaining a deeper understanding of dynamic linking in `dlbox`. 🚀