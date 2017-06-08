#ifndef _MYLIB_H_2_
#define _MYLIB_H_2_

void funct();
void set_shared(void *, void *, void *);
void **l1_s, **l1_e;
void *(*shared_func)(char *func_name, void **, void **);

#endif