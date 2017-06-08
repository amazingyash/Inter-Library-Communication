#ifndef _MYLIB_H_
#define _MYLIB_H_

void callfunc();
void set_shared(void *fun);
void *(*interface_func)(char *);

#endif