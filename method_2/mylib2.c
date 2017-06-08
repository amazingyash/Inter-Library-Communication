#include "structure.h"
#include "mylib2.h"
#include <stdio.h>
#include <stdlib.h>
#include <dlfcn.h>
#include <string.h>
#include <sys/types.h>
#include <unistd.h>
#include <sys/mman.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <errno.h>

void set_shared_B(void *b, unsigned int size)   // this sets up the shared functions
{
	interface_b_main = (unsigned int (*)(char *))b;
	sizeA = size;
	lemon = 6;
	printf("interface_b_main set\n");
}

void funcB()  // this is the function which calls a function in library 1
{
	unsigned int offset = interface_b_main("func_A");  // getting the offset from the shared function in trusted code
	int fd = shm_open("/functions_A", O_RDWR, 0777);  // opening the shared memory of library 1
	if(fd == -1)
	{
		fprintf(stderr, "Open failed:%s\n", strerror(errno));
		return;
	}
	void *addr = mmap(NULL,sizeA, PROT_READ | PROT_WRITE | PROT_EXEC, MAP_SHARED, fd, 0);// mapping the shared memory to its own address
	if(addr == (void *)-1)
	{
		fprintf(stderr, "mmap failed:%s\n", strerror(errno));
		return;
	}
	void *temp = addr + offset;  // adding the offset to get the location of function in the mapping
	int (*temp_func)(int) = (int (*)(int))temp; // casting the pointer to appropriate format
	int k = 34, j;
	j = temp_func(k);  // calling the function
	printf("Return value from A in B%d\n", j);

}

void changel()
{
	lemon = 18;
}

int func_B(linkedlist *l) // test function
{
	lemon = 8;
	return l->data + lemon;
}