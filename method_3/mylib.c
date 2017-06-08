#include "mylib.h"
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

void set_shared(void *fun)
{
	interface_func = (void *(*)(char *))fun;
}

void callfunc()
{
	struct timespec t1,t2;
	int k = 45, l;
	int (*func)(int);
	clock_gettime(CLOCK_MONOTONIC,&t1);
	func = interface_func("funct");  // gets function pointer in the new mapping from the trusted code
	l = func(k);
	clock_gettime(CLOCK_MONOTONIC,&t2);
	printf("return value = %d and time taken %ld\n", l, t2.tv_nsec - t1.tv_nsec);
	FILE *log_file;
	log_file = fopen("log.txt", "a");
	fprintf(log_file, "%ld\n", t2.tv_nsec - t1.tv_nsec);
}