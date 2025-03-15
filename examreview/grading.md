# **Return-to-libc (ret2libc) Grading Criteria (Total: 100 Points)**

This grading rubric evaluates the implementation of a **return-to-libc (ret2libc) attack**, focusing on **offset calculation, libc function address retrieval, ROP gadget selection, and successful exploitation**.  

---

## **Grading Breakdown**  

### **1️⃣ Calculating the Buffer Offset and Overwriting the Return Address (30 Points)**  

✅ **Goal**: Identify the **offset between the buffer and the return address** and successfully overwrite the return address.  

| **Criteria** | **Points** | **Description** |
|-------------|-----------|----------------|
| **Finding the buffer location** | 5 | Determine the exact buffer address in memory. |
| **Finding the return address location** | 5 | Identify the return address using `gdb` or other debugging tools. |
| **Correctly calculating the buffer-to-return address offset** | 5 | Compute the precise offset between the buffer and the return address. |
| **Constructing the payload with the correct offset** | 5 | Use `b"A" * offset` or `bytearray()` to ensure correct overwriting. |
| **Overwriting the return address properly** | 10 | Ensure the payload successfully replaces the return address. |

---

### **2️⃣ Finding libc and Function Addresses (30 Points)**

✅ **Goal**: Retrieve **libc base address, system(), exit(), and /bin/sh address** correctly.  

| **Criteria** | **Points** | **Description** |
|-------------|-----------|----------------|
| **Finding the libc base address** | 10 | Use `info proc mappings` or `ldd` to obtain the libc base address. |
| **Correctly computing system(), exit(), setuid(), and /bin/sh addresses** | 20 | Use `p system`, `p exit`, or `search "/bin/sh"` to verify correctness. |

---

### **4️⃣ Selecting the Correct ROP Gadget and Finding Its Address (20 Points)** 

✅ **Goal**: Identify the appropriate **ROP gadgets** and correctly compute their positions in libc.  

| **Criteria** | **Points** | **Description** |
|-------------|-----------|----------------|
| **Finding the correct ROP gadget (pop rdi; ret, etc.)** | 10 | Use `ROPgadget` or `ROPgadget --binary libc.so.6` to locate `pop rdi; ret`. |
| **Computing the gadget’s correct address in libc** | 10 | Ensure the correct calculation of gadget offsets relative to libc. |

---

### **5️⃣ Successful Exploit Execution (20 Points)**  

✅ **Goal**: The final payload **executes system("/bin/sh") successfully**, achieving a working shell.  

| **Criteria** | **Points** | **Description** |
|-------------|-----------|----------------|
| **Payload structure and correctness** | 10 | Ensure the ROP chain follows the correct structure, using `pop rdi; ret` before `system("/bin/sh")`. |
| **Successful execution of system("/bin/sh")** | 10 | The exploit successfully spawns an interactive shell. |

---

## **Final Score Summary**

| **Category** | **Points** |
|-------------|-----------|
| Calculating Buffer Offset & Overwriting Return Address | 30 |
| Finding libc & Function Addresses | 30 |
| Selecting & Calculating ROP Gadget Address | 20 |
| Successfully Executing the Exploit | 20 |
| **Total** | **100** |
