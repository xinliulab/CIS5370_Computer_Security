CC := gcc
CFLAGS := -O2 -Wall -std=gnu11 -g
TARGET := fsrecov
SRCS := fsrecov.c
OBJS := $(SRCS:.c=.o)

.PHONY: all clean

all: $(TARGET)

$(TARGET): $(OBJS)
	$(CC) $(CFLAGS) -o $@ $^

clean:
	rm -f $(TARGET) $(OBJS)
