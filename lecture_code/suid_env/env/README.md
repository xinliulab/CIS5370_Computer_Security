# README: Running and Debugging the Environment Variable Programs  

## Overview  
This repository contains three C programs that demonstrate different aspects of environment variable handling in Linux:  

1. **`env.c`** - Observes how environment variables are passed to a process and when `environ` is initialized.  
2. **`passenv.c`** - Demonstrates how different environment variable sets can be passed to a new process using `execve`.  
3. **`print_pwd.c`** - Retrieves and prints the current working directory using the `PWD` environment variable.  

---

## Compilation  

To compile all programs, use the `gcc` compiler:  

```sh
gcc -o env env.c
gcc -o passenv passenv.c
gcc -o print_pwd print_pwd.c
```

If you only want to compile a specific program, run the corresponding `gcc` command above.

---

## Running and Observing Behavior  

### 1. `env.c` – Observing How `environ` Gets Assigned  

This program prints the addresses of `environ` and `envp`, allowing us to observe when `environ` gets assigned.  

#### Run:  
```sh
./env
```
#### Expected Output Example:  
```
0x7ffcd1234567
0x7ffcd1234567
```

#### Debugging with GDB:  
To observe when `environ` is set, use `gdb`:  

```sh
gdb ./env
starti
si
p environ
```
- This should show `environ = 0x0`, meaning it has not yet been assigned.  

To track when `environ` is modified:  

```sh
watch environ
c
```
- The program will stop when `environ` is assigned.  
- You should see that **libc** is responsible for setting `environ`.  

---

### 2. `passenv.c` – Passing Environment Variables to a New Process  

This program launches the `/usr/bin/env` command using different environment settings based on an argument.

#### Run with Different Arguments:  
```sh
./passenv 1   # Pass no environment variables
./passenv 2   # Pass a custom environment (AAA=aaa, BBB=bbb)
./passenv 3   # Pass all current environment variables
```
- `./passenv 1` should result in `/usr/bin/env` printing nothing.  
- `./passenv 2` should display only `AAA=aaa` and `BBB=bbb`.  
- `./passenv 3` should display all environment variables inherited from the current shell.  

---

### 3. `print_pwd.c` – Retrieving the `PWD` Environment Variable  

This program fetches and prints the current working directory using `getenv("PWD")`.  

#### Run:  
```sh
./print_pwd
```
#### Expected Output Example:  
```
Present working directory is: /home/user
```
- If `PWD` is not set, no output will be shown.  

#### Modify Environment and Run Again:  
```sh
export PWD="/tmp"
./print_pwd
```
- Now the output should reflect `/tmp`.  

---

## Summary  

- **`env.c`** shows when `environ` gets assigned using `gdb`.  
- **`passenv.c`** demonstrates different ways to pass environment variables to a new process using `execve`.  
- **`print_pwd.c`** retrieves and prints the `PWD` environment variable.  

These programs help in understanding how environment variables are managed at different stages of process execution.  

---

This `README.md` should help your students compile, run, and debug the programs efficiently! 🚀