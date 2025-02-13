/*
ls =l libbloat.so
objdump -d libbloat.so | less

LD_LIBRARY_PATH=. ./bloat
pmap | grep libbloat.so
*/

void bloat() {
    // 100M of nops
    asm volatile(
        ".fill 104857600, 1, 0x90"
    );
}
