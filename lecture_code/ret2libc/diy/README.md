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

Inspect the preprocessed assembly file:
```bash
gcc -E main.S
```

Read symbol table from a dynamic library:
```bash
./dlbox readdl libc.dl
```

Compile an object file with position-independent code:
```bash
gcc -m64 -fPIC -c main.S
```

Disassemble an object file:
```bash
objdump -d libhello.o
```

View exported symbols from `libhello.dl`:
```bash
./dlbox readdl libhello.dl
```

## Understanding the Code

### Key Components

- **dlbox.c**
  - Implements a custom dynamic linker and loader
  - Loads `.dl` files into memory using `mmap()`
  - Resolves symbols using a manually managed lookup table

- **dl.h**
  - Defines the structure of a `.dl` file, including headers and symbol records
  - Implements macros for importing and exporting symbols in assembly

- **main.S, libc.S, libhello.S**
  - Example assembly source files demonstrating how symbols are defined and referenced

### Learning Outcomes

By working with `dlbox`, students will:
- Understand how dynamic linking works at a low level
- Learn how symbols are resolved in a dynamically linked program
- Explore how shared libraries are loaded and executed without relying on the OS linker
- Gain hands-on experience with assembly language and symbol resolution