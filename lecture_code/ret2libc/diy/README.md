**dlbox**: An "ultra-minimal" dynamic linker and loader that invokes dynamically linked symbols through a lookup table. Interested students can explore the details of ELF dynamic linking and loading after class.

Try it out with: 

./dlbox interp main.dl

gcc -E main.S

./dlbox readdl libc.dl

gcc -m64 -fPIC -c main.S

objdump -d libhello.o

cat libhello.txt

./dlbox readdl libhello.dl