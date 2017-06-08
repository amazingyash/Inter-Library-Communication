#include "structure.h"
#include "mylib2.h"
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

void set_shared(void *a, void *b, void *c)
{
	shared_func = (void *(*)(char *func_name, void **, void **))a;
	l1_s = b;
	l1_e = c;
	printf("in set shared %p %p\n", l1_s[0], l1_e[0]);
	printf("ssssShared func set\n");
}

void funct()
{
	struct timespec t1,t2;
	printf("entered funct\n");
	int k;
	char *l = "yash";
	linkedlist *ret;
	linkedlist *head;
	head = malloc(sizeof(linkedlist));
	head->data = 313;
	linkedlist *n;
	n = malloc(sizeof(linkedlist));
	n->data = 444;
	head->next = n;
	n->next = NULL;
	clock_gettime(CLOCK_MONOTONIC,&t1);
	void *func = shared_func("func", l1_s, l1_e);
	linkedlist *(*func_call)(linkedlist *) = (linkedlist *(*)(linkedlist *))func;
	ret = func_call(head);
	clock_gettime(CLOCK_MONOTONIC,&t2);
	printf("return value %d and time taken %ld\n", ret->data , t2.tv_nsec - t1.tv_nsec);
	FILE *in;
	in = fopen("log.txt","a");
	fprintf(in, "%ld\n", t2.tv_nsec - t1.tv_nsec);
	fclose(in);
	// printf("return value of function in library 1 %d\n", func_call(34));
}