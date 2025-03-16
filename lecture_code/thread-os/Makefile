NAME := kernel
SRCS := $(shell find . -name "*.c")
export ARCH := x86_64-qemu
AM_HOME := /home/ubuntu/CIS5370_Computer_Security/lecture_code/abstract-machine
include $(AM_HOME)/Makefile

debug:
	qemu-system-x86_64 -s -S build/kernel-x86_64-qemu
