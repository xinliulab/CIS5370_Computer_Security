/*
gcc -g a.c
file a.out
readelf -a a.out

Relocation section '.rela.plt' at offset 0x610 contains 1 entry:
  Offset          Info           Type           Sym. Value    Sym. Name + Addend
000000003fd0  000400000007 R_X86_64_JUMP_SLO 0000000000000000 exit@GLIBC_2.2.5 + 0

gdb ./a.out
starti
info inferiors

pmap 187976
python3

hex(0x000000003fd0 + 0x0000555555554000)

watch *0x555555557fd0
rwatch *0x555555557fd0

p/x *0x555555557fd0
*/

#include <stdlib.h>

int main()
{
	exit(0);
}
