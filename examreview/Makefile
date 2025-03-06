all:
	gcc -o stack stack.c -no-pie -g -fno-stack-protector -fno-stack-clash-protection -fcf-protection=none
	sudo chown root stack
	sudo chmod 4755 stack
