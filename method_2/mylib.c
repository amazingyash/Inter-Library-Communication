#include "structure.h"
#include "mylib.h"
#include <stdio.h>
#include <stdlib.h>
#include <dlfcn.h>
#include <sys/types.h>
#include <unistd.h>
#include <sys/mman.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <errno.h>
#include <string.h>
#include <time.h>

void set_shared_A(void *b, unsigned int size)  // sets up the shared function and the size of library 2
{
	interface_a_main = (unsigned int (*)(char *))b;
	sizeB = size;
	printf("size received by A %u\n", sizeB);
	printf("interface_a_main set\n");
}

int func_A(int ar) // test function
{
	return ar + 3;
}

void funcA() // calls a function in library 2
{
	struct timespec t1,t2;
	linkedlist *j;
	linkedlist *k;
	k = malloc(sizeof(linkedlist));
	k->data = 313;
	linkedlist *ttt;
	ttt = malloc(sizeof(linkedlist));
	ttt->data = 444;
	ttt->next = NULL;
	k->next = ttt;
	class cl;
	cl.stren = 313;
	cl.name = "A1";
	char *arg1 = "yash";
	int ret_val;
	clock_gettime(CLOCK_MONOTONIC,&t1);
	unsigned int offset = interface_a_main("func_B");  // gets an offset from the trusted code
	int fd = shm_open("/functions_B", O_RDWR, 0777);   // opens the shared memory containing code of library 2
	if(fd == -1)
	{
		fprintf(stderr, "Open failed:%s\n", strerror(errno));
		return;
	}
	void *addr = mmap(NULL,sizeB, PROT_READ | PROT_EXEC, MAP_SHARED, fd, 0); // maps the shared library to own address space
	if(addr == (void *)-1)
	{
		fprintf(stderr, "mmap failed:%s\n", strerror(errno));
		return;
	}
	void *temp = addr + offset; // adds the offset to get function address in the offset
	int (*temp_func)(linkedlist *) = (int (*)(linkedlist *))temp;    //casting it to the appropriate function pointer
	ret_val = temp_func(k);                                  // calls the function
	clock_gettime(CLOCK_MONOTONIC,&t2);
	printf("Return value from B in A : %d and time taken %ld\n", ret_val, t2.tv_nsec - t1.tv_nsec);
	ret_val = temp_func(k);
	printf("return value second time %d\n", ret_val);
	FILE *in = fopen("log.txt","a");
	fprintf(in, "%ld\n", t2.tv_nsec - t1.tv_nsec);
	fclose(in);
}