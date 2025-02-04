#include <stdio.h>
#include <unistd.h>
#include <sys/syscall.h>


int main(){
//void _start() {
	//printf("Hello, World!\n");
	//while(1);
	syscall(SYS_exit,5370);

}
