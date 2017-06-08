#include <stdio.h>
#include <stdlib.h>
#include <dlfcn.h>
#include <string.h>
#include <sys/types.h>
#include <unistd.h>
#include <sys/mman.h>

void *handle, *new_map, *handle2;
unsigned int size;

void *two2one(char *func_name, void **l1_s, void **l1_e)
{
	posix_memalign(&new_map, 4096, size);  //allocates the page aligned memory, rounding off size to the nearest multiple of page size
	if (mprotect(new_map, size, PROT_READ|PROT_EXEC|PROT_WRITE) == -1) { // changes the protection permissions of the new memory
      perror ("mprotect"); 
  	}
  	// printf("%p\n", new_map);
  	// printf("new_map:%p\nl1_s[0]:%p\nl1_e[0]:%p\n", new_map, l1_s[0], l1_e[0]);
	memcpy(new_map, l1_s[0], l1_e[0] - l1_s[0]); // copying data from library mappings to here
	// printf("entered\n");
	void *temp;
	// printf("Segment1 copied successfully\n");
	temp = new_map + (l1_e[0] - l1_s[0]);
	
	// skipped segment 2 because we don't have read permission. We don't need it perhaps.

	temp = temp + (l1_e[1] - l1_s[1] - 999);
	memcpy(temp ,l1_s[2] , l1_e[2] - l1_s[2]);
	// printf("Segment3 copied successfully\n");
	temp = temp + (l1_e[2] - l1_s[2]);
	memcpy(temp + 1000,l1_s[3] + 1000, l1_e[3] - l1_s[3] - 1000);
	// printf("Segment4 copied successfully\n");
	// printf("New mapping made at %p\n", new_map);
	temp = dlsym(handle,func_name);
	unsigned int offset = temp - new_map;
	void *new_ptr = new_map + offset;
	return new_ptr;
}


int main(int argc, char const *argv[])
{
	printf("%d %d\n", getpid(), getpagesize());
	if(argc < 3)
	{
		printf("Usage : <executable> <library 1> <library 2>\n");
		exit(1);
	}
	char lib_path[30];
	sprintf(lib_path,"./%s",argv[1]);
	printf("%s\n", lib_path);
	handle = dlopen(lib_path,RTLD_LAZY);
	if(!handle)
	{
		fputs(dlerror(),stderr);
		exit(1);
	}
	sprintf(lib_path,"./%s",argv[2]);
	handle2 = dlopen(lib_path,RTLD_LAZY);
	if(!handle2)
	{
		fputs(dlerror(),stderr);
		exit(1);
	}
	char map_path[20];
	void *l1_seg_s[3], *l1_seg_e[3];
	sprintf(map_path,"/proc/%d/maps",getpid());
	FILE *in;
	in = fopen(map_path,"r");
	if(in == NULL)
	{
		printf("error in opening mapping\n");
		exit(1);
	}
	char line[200];
	int flag = 0;
	char *token;
	while(feof(in) == 0)
	{
		fgets(line,200,in);
		if(strstr(line,argv[1]) != NULL)
		{
			printf("line in proc : %s\n",line);
			token = strtok(line," ");
			printf("token : %s|\n", token);
			sscanf(token,"%p-%p",&l1_seg_s[flag], &l1_seg_e[flag]);
			printf("read addresses %p - %p\n", l1_seg_s[flag], l1_seg_e[flag]);
			flag++;
		}
	}
	// char c;
	// c = getchar();
	void (*setshared)(void *, void *, void *) = dlsym(handle2,"set_shared");
	setshared((void *)two2one, l1_seg_s, l1_seg_e);
	printf("\n\n\n");
	for(int i = 0 ; i < 4 ; i++)
	printf("%p - %p\n", l1_seg_s[i], l1_seg_e[i]);
	size = (int)(l1_seg_e[3] - l1_seg_s[0]);
	printf("size of allocation(bytes) : %u\n", size);

	
	// void *func_ptr;
	// void *func_ptr_new;
	// func_ptr = dlsym(handle,"func");
	// printf("address of func :%p\n", func_ptr);
	// printf("offset from segment :%ld\n", (func_ptr - l1_seg_s[0]));
	// func_ptr_new = (func_ptr - l1_seg_s[0]) + new_map;
	// printf("address in new mapping %p\n", func_ptr_new);
	// // printf("%d %d\n", (*((int *)func_ptr + 147*4)), (*((int *)func_ptr_new+ 149*4)));
	// int (*functi)(void) = (int (*)(void))func_ptr_new;
	// printf("from the new mapping %d\n",functi());
	// printf("l1_seg_s[0] new_map : %p %p\n", l1_seg_s[0], new_map);
	// printf("off : %u\n", off);
	// printf("func_ori new_func : %p %p\n", dlsym(handle,"func"), new_map + off);
	// printf("int vals : %d %d\n", (*(int *)new_map + 1648), (*(int *)l1_seg_s[0] + 1648));
	void *temp_ptr = two2one("func",l1_seg_s,l1_seg_e);
	// printf("%p\n", new_map + off);
	// printf("%p\n", new_map + 1648);
	// printf("%p\n", temp_ptr);
	// printf("temp ptr %d\n", (*(int *)temp_ptr));
	// printf("new_map + off %d\n", (*(int *)new_map + off));
	printf("sg\n");
	void (*funct_ptr)(void) = dlsym(handle2,"funct");
	printf("hhhhhh\n");
	funct_ptr();
	free(new_map);
	dlclose(handle);
	return 0;
}