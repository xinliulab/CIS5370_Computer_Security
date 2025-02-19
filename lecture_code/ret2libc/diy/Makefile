# Compiler
CC = gcc
CFLAGS = -Wall -Wextra -O2 -g

# Object Files
DLBOX = dlbox
DL_FILES = libhello.dl libc.dl main.dl
TXT_FILES = libhello.txt libc.txt main.txt
OBJ_FILES = libhello.o libc.o main.o

# Defauly Object
all: check_nasm $(DLBOX) $(DL_FILES) $(TXT_FILES) clean_o

# Check for ndisasm (if absent, install)
check_nasm:
	@if ! command -v ndisasm >/dev/null 2>&1; then \
	    echo "Installing nasm..."; \
	    sudo apt update && sudo apt install -y nasm || { \
	        echo "Error: Failed to install nasm. Please install it manually using:"; \
	        echo "  sudo apt update && sudo apt install nasm -y"; \
	        exit 1; \
	    }; \
	else \
	    echo "nasm is already installed."; \
	fi

# .dl
libhello.dl: libhello.S $(DLBOX)
	./dlbox gcc $<

libc.dl: libc.S $(DLBOX)
	./dlbox gcc $<

main.dl: main.S $(DLBOX)
	./dlbox gcc $<

# disassembly
libhello.txt: libhello.dl
	./dlbox objdump $< > $@

libc.txt: libc.dl
	./dlbox objdump $< > $@

main.txt: main.dl
	./dlbox objdump $< > $@

# dlbox
$(DLBOX): dlbox.c
	$(CC) $(CFLAGS) $< -o $@

# clean
clean:
	rm -f $(DLBOX) $(DL_FILES) $(TXT_FILES) $(OBJ_FILES)

# clean .o 
clean_o:
	rm -f $(OBJ_FILES)
