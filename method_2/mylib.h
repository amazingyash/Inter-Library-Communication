#ifndef _MYLIB_H_
#define _MYLIB_H_

int func_A(int); // test function
void set_shared_A(void *, unsigned int); // this sets up the shared function
unsigned int (*interface_a_main)(char *); // this is the shared function pointer
void funcA(); // calls a function in library 2
unsigned int sizeB; // size of library 2

#endif