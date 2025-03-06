# ROP Chain and Gadget Usage in 64-Bit Exploits

On a 64-bit system, function arguments are passed in registers, while the actual data (and our ROP chain) is stored on the stack. The code that we “call” (our gadgets and library functions) resides in libc. Keep in mind that as the program in libc executes one instruction at a time, the stack pointer (rsp) is moving on the stack in 8‑byte increments.

For example, if the stack address of the "ret" instruction in the vulnerable function is **0x7fffffffd8b8**, then:

- In reality, this gadget's purpose is to “skip over” a portion of stack data so that rsp points to the effective part of the ROP chain that we constructed.
- Since this gadget in libc not only executes `add $0x90, %rsp` to adjust rsp but is also followed by several pop instructions (like `pop %rbx`, `pop %r12`, `pop %rbp`, etc.), each pop moves rsp by another 8 bytes.
- Therefore, we fill the payload with “B” bytes as padding to ensure that after these instructions run, the final ret will jump to the effective address we want to execute.

For instance, the program jumps into libc to run `add $0x90, %rsp` but rsp still points to the stack. The sequence is roughly:

- `add $0x90, %rsp` → rsp becomes: **0x7fffffffd8c0**
- `pop %rbx` → rsp becomes: **0x7fffffffd950**
- `pop %r12` → rsp becomes: **0x7fffffffd958**
- `pop %rbx` → rsp becomes: **0x7fffffffd960**
- `ret` → rsp becomes: **0x7fffffffd968**


## WHY `add $0x90, %rsp`

In our buffer overflow scenario, we use strcpy to overwrite the return address. However, strcpy stops copying when it encounters a null byte (0x00). This means that if our payload contains null bytes too early, the overflow gets truncated. Additionally, after the initial overflow, the badfile may be re-read or reprocessed, which further complicates things.

Because of this, the overwritten return address cannot simply point to the very next address. Instead, it must jump far enough to skip over:
- The "A" string (which fills the initial buffer) after the return address, 
- And the value of the second occurrence of the return address.

By using a gadget that performs `add $0x90, %rsp`, we effectively adjust the stack pointer so that after all the intermediate gadget operations, control lands exactly within the "B" string. This "B" padding region is where our effective ROP chain begins.

In gdb, after the strcpy executes, you can use:

```
x/400xb 0x7fffffffd840
```

to inspect 400 bytes of memory starting at address 0x7fffffffd840 and observe the stack contents.



## Purpose of the Zeroing Operation for setuid()

We need to supply an argument to setuid() – specifically, we want to pass 0 – and we use sprintf to write a 0 into a specific memory location that will later be used as the parameter for setuid().

- On a 64-bit system, an argument occupies 8 bytes.  
- However, sprintf writes one byte (a character) at a time.  
- To write an 8‑byte 0, we call sprintf 8 times – each call writes one 0 byte.

The code segment below explains this mechanism:

```python
for i in range(0, 8):
    payload += to_bytes(pop_rsi_rbp_addr)
    payload += to_bytes(null_addr)
    payload += to_bytes(0xdeadbeefdeadbeef)
    payload += to_bytes(pop_rdi_addr)
    payload += to_bytes(target_addr + i)
    payload += to_bytes(sprintf_addr)
```

### Explanation:

1. **First Gadget (pop_rsi_rbp_addr):**  
   - `pop rsi` – Pops a value from the stack into rsi. At this moment, rsp points to **null_addr**, so rsi is set to null_addr.  
   - `pop rbp` – Pops the next value into rbp. This is a dummy placeholder (`0xdeadbeefdeadbeef`) with no actual significance.  
   - `ret` – Now rsp points to the next gadget, which is `pop_rdi_addr`.

2. **Second Gadget (pop_rdi_addr):**  
   - `pop rdi` – Pops a value into rdi. At this point, rsp points to `target_addr + i`.  
   - `ret` – Now rsp points to the address of sprintf.

3. **Calling sprintf:**  
   The control transfers to sprintf, which is called with the parameters:
   ```c
   sprintf(target_addr + i, null_addr);
   ```
   Since the content at null_addr is an empty string (or at least ends with a 0), sprintf writes a 0 byte into the memory at target_addr + i.

*Note:* In libc, the string "/bin/sh" is null-terminated. Thus, the memory right after "/bin/sh" contains a 0 byte. This is why we set:  
```python
null_addr = binsh_addr + len("/bin/sh")
```
This provides us with an address that holds a null byte, which is used as the format string in sprintf. You cannot simply write "00" or "null" as a string because what we need is an address that points to a memory location containing a 0 byte. In principle, any address containing a 0 byte can be used.

---

## Explanation of the setuid() ROP Frame

```python
payload += to_bytes(pop_rdi_addr)
payload += to_bytes(0xbeefbeefdeaddead)
payload += to_bytes(setuid_addr)
```

### Expanded in Assembly Terms:

- **Gadget: pop rdi; ret**  
  When this gadget executes:
  - `pop rdi` – The next 8 bytes (which is 0xbeefbeefdeaddead) are popped from the stack into rdi.
  - `ret` – The next 8 bytes, which is the address of setuid(), are popped and control is transferred there.

Thus, it appears as though we are calling:
```c
setuid(0xbeefbeefdeaddead);
```
However, due to our earlier sprintf loop (or similar zeroing operation), the memory at the target address (which should be used as the setuid parameter) is overwritten with 0. Therefore, when setuid() is finally called, the value in rdi (read from that memory location) becomes 0.

---

## Explanation of the system() and exit() ROP Frame

```python
payload += to_bytes(pop_rdi_addr)
payload += to_bytes(binsh_addr)
payload += to_bytes(system_addr)
payload += to_bytes(pop_rdi_addr)
payload += to_bytes(0)
payload += to_bytes(exit_addr)
```

### Expanded in Assembly Terms:

- First, the gadget `pop rdi; ret` loads `binsh_addr` into rdi, then `ret` transfers control to system(), resulting in the call:
  ```c
  system("/bin/sh");
  ```
- Immediately after, another `pop rdi; ret` gadget loads 0 into rdi, and `ret` transfers control to exit(), resulting in:
  ```c
  exit(0);
  ```

---

# Summary of Key Points

1. **64-bit Data Transfer:**  
   - A 64-bit system uses registers to pass function arguments.
   - The data (including our ROP chain) is stored on the stack.
   - The gadgets (instructions to set registers and adjust the stack) reside in libc.

2. **Stack and Gadget Operation:**  
   - The stack address of the saved `ret` in `foo()` is 0x7fffffffd8b8.
   - The purpose of the gadget (e.g., one that executes `add $0x90, %rsp`) is to "skip over" a portion of stack data so that rsp points to the effective part of the ROP chain we constructed.
   - This gadget, located in libc, not only executes `add $0x90, %rsp` but is followed by several pop instructions (e.g., `pop %rbx`, `pop %r12`, `pop %rbp`, etc.). Each pop moves rsp by 8 bytes.
   - We fill the payload with “B” bytes as padding to ensure that after these instructions run, the final ret transfers control to the correct effective address.

   For example, the program jumps into libc to run `add $0x90, %rsp`, but rsp still points to the stack. The progression is:
   - `add $0x90, %rsp` → rsp becomes 0x7fffffffd8c0.
   - `pop %rbx` → rsp becomes 0x7fffffffd950.
   - `pop %r12` → rsp becomes 0x7fffffffd958.
   - `pop %rbx` → rsp becomes 0x7fffffffd960.
   - `ret` → rsp becomes 0x7fffffffd968.

3. **Using sprintf to Zero Out an Argument for setuid():**  
   - We need to pass 0 as the argument to setuid().
   - An argument occupies 8 bytes in a 64-bit system, but sprintf writes one byte at a time.
   - Therefore, we call sprintf 8 times (once for each byte) to write a 0 into the target memory address.
   - In the loop, the first gadget (`pop rsi; pop rbp; ret`) sets rsi to `null_addr` (which points to a null byte), and the second gadget (`pop rdi; ret`) sets rdi to `target_addr + i`.
   - The sprintf call writes a 0 byte to `target_addr + i`.
   - After 8 iterations, the entire 8-byte area at target_addr is filled with 0. This region is then used as the parameter for setuid().

4. **ROP Frames for setuid(), system(), and exit():**  
   - For setuid():  
     ```python
     payload += to_bytes(pop_rdi_addr)
     payload += to_bytes(0xbeefbeefdeaddead)  # Placeholder
     payload += to_bytes(setuid_addr)
     ```
     Even though the placeholder value 0xbeefbeefdeaddead is placed in the payload, our earlier sprintf calls overwrite the target address with 0. Thus, setuid() receives 0.
   - For system() and exit():  
     ```python
     payload += to_bytes(pop_rdi_addr)
     payload += to_bytes(binsh_addr)
     payload += to_bytes(system_addr)
     payload += to_bytes(pop_rdi_addr)
     payload += to_bytes(0)
     payload += to_bytes(exit_addr)
     ```
     This sets up the calls `system("/bin/sh")` and `exit(0)`.


