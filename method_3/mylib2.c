#include "mylib2.h"
#include <stdio.h>
#include <stdlib.h>

void changek()
{
	k = 34;
}

int funct(int a)
{
	k = 45;
	return a + 3;
}