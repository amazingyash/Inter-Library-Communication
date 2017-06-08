#ifndef _MYLIB_H_2_
#define _MYLIB_H_2_

void funcB(); // function that calls a function library 1
void set_shared_B(void *, unsigned int); // this sets up the shared function and gives the size of the libary 1
int func_B(linkedlist *); // test function
unsigned int (*interface_b_main)(char *); // shared function pointer
unsigned int sizeA; // size of library 1 mapping
void changel();
int lemon;
#endif