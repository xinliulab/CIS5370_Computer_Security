**Shared Library Test**: We launch 100 functions, each 100M in size, and test whether multiple independently started processes retain only one copy of the shared library's code by observing the memory usage in the system.

Tyr it with:
make

ls -l libbloat.so

objdump -d libbloat.so | less

LD_LIBRARY_PATH=. ./bloat

./run

ps aux

pmap | grep libbloat.so
*/