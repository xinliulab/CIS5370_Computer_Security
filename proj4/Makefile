# ------------------------------------------------------------------
# Unified Makefile — builds **64‑bit** fsrecov only
# ------------------------------------------------------------------
#  • Combines the per‑lab wrapper and the common template.
#  • Removes all 32‑bit and shared‑library targets for simplicity.
# ------------------------------------------------------------------

NAME    := fsrecov           # final binary name (fsrecov)
MODULE  := M6                # exported so other scripts can pick it up
export MODULE

# ------------------------------------------------------------------
# Source / dependency lists (current directory only)
# ------------------------------------------------------------------
SRCS := $(shell find . -maxdepth 1 -name "*.c")
DEPS := $(shell find . -maxdepth 1 -name "*.h") $(SRCS)

# ------------------------------------------------------------------
# Compiler / linker flags
# ------------------------------------------------------------------
CFLAGS  := -O1 -std=gnu11 -ggdb -Wall -Werror \
           -Wno-unused-result -Wno-unused-value -Wno-unused-variable
LDFLAGS := -lm

# ------------------------------------------------------------------
# Phony targets
# ------------------------------------------------------------------
.PHONY: all clean

# Default target — build the 64‑bit executable
all: $(NAME)

# ------------------------------------------------------------------
# Build rule (64‑bit only)
# ------------------------------------------------------------------
$(NAME): $(DEPS)
	gcc -m64 $(CFLAGS) $(SRCS) -o $@ $(LDFLAGS)

# ------------------------------------------------------------------
# House‑keeping
# ------------------------------------------------------------------
clean:
	rm -f $(NAME)
