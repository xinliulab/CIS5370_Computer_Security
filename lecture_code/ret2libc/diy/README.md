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