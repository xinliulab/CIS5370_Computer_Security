# Shared Library Memory Test

## Overview
This test evaluates whether multiple independently launched processes can share a single copy of a large shared library (`libbloat.so`). By running 100 processes that each use `libbloat.so`, we analyze system memory behavior and check if excessive memory usage leads to an Out of Memory (OOM) condition.

## Purpose
The test aims to:

- Generate a large shared library (`libbloat.so`) where each function is 100MB
- Launch 100 separate processes that link to `libbloat.so`
- Observe whether only one copy of the shared library is retained in memory across processes
- Monitor memory usage to verify if shared libraries truly prevent redundant memory allocation

## Setup and Execution

### 1. Build the Shared Library and Executable

```bash
make
```

### 2. Verify the Shared Library

Check the file size to confirm it's large:
```bash
ls -l libbloat.so
```

Disassemble the shared library to inspect function sizes:
```bash
objdump -d libbloat.so | less
```

### 3. Run the Test

Start 100 processes using the provided script:

```bash
./run
```

## Observing Memory Usage

### Check Active Processes
Open a new terminal
```bash
ps aux | grep bloat
```

### Verify Shared Library Mapping
To check if all processes share `libbloat.so`:
```bash
pmap <pid> | grep libbloat.so
```
Replace `<pid>` with the actual process ID.

### Monitor System Memory
Monitor free memory:
```bash
free -h
```

Monitor virtual memory statistics:
```bash
vmstat 1
```

## Expected Outcome

The test has two possible outcomes:

1. **Success**: If shared libraries work correctly:
   - Memory usage should not grow linearly with process count
   - System should maintain one copy of `libbloat.so` in memory

2. **Failure**: If the system fails to share the library:
   - Memory usage will increase significantly
   - May trigger an Out of Memory (OOM) error

This test helps determine whether shared libraries effectively optimize memory usage in multi-process environments.