#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/mman.h>
#include <fcntl.h>
#include <errno.h>
#include <dlfcn.h>
#include <sys/types.h>
#include <unistd.h>

void *srcA, *srcB;
unsigned int sizeA, sizeB;
void *l1_seg_s[3], *l1_seg_e[3];
void *l2_seg_s[3], *l2_seg_e[3];
void *handle1, *handle2, *addrB, *addrB2;

void *A2main(char *func_name)
{
	// void *temp = l2_seg_s[3];
	addrB2 = mmap(NULL, sizeB, PROT_READ | PROT_WRITE | PROT_EXEC, MAP_SHARED | MAP_ANONYMOUS, -1, 0);
	// l2_seg_s[3] = temp;
	if(addrB == (void *)-1)
	{
		fprintf(stderr, "mmap failed%s\n", strerror(errno));
		return (void *)-1;
	}
	//copying the data in the new mapping
	void *temp;
	// printf("libB : Segment1 copying : %p -> %p\n", addrB2, addrB2 + (l2_seg_e[0] - l2_seg_s[0]));
	memcpy(addrB2, l2_seg_s[0], l2_seg_e[0] - l2_seg_s[0]);
	// printf("libB : Segment1 copied successfully : %p -> %p\n", addrB2, addrB2 + (l2_seg_e[0] - l2_seg_s[0]));
	temp = addrB2 + (l2_seg_e[0] - l2_seg_s[0]);
	// printf("temp : supposed to be value %p %p\n", temp, addrB2 + (l2_seg_e[0] - l2_seg_s[0]));
	temp = temp + (l2_seg_e[1] - l2_seg_s[1]);
	// printf("libB : Segment3 copying : %p -> %p\n", temp, temp + (l2_seg_e[2] - l2_seg_s[2]));
	memcpy(temp ,l2_seg_s[2] , l2_seg_e[2] - l2_seg_s[2]);
	// printf("libB : Segment3 copied successfully : %p -> %p\n", temp, temp + (l2_seg_e[2] - l2_seg_s[2]));
	temp = temp + (l2_seg_e[2] - l2_seg_s[2]);
	// printf("libB : Segment4 copying : %p -> %p\n", temp + 1000, temp + (l2_seg_e[3] - l2_seg_s[3]));
	memcpy(temp + 1000,l2_seg_s[3] + 1000, l2_seg_e[3] - l2_seg_s[3] - 1000);
	// printf("libB : Segment4 copied successfully : %p -> %p\n", temp + 1000, temp + (l2_seg_e[3] - l2_seg_s[3]));
	// printf("address of new mapping of library B %p\n", addrB2);
	unsigned int off = (dlsym(handle2,func_name) - srcB);
	void *new_func = addrB2 + off;
	return new_func;
}

int main(int argc, char const *argv[])
{
	printf("l2_seg_s[3] and addrB and handle2 %p %p %p\n", &l2_seg_s[3], &addrB, &handle2);
	printf("%d %d\n", getpid(), getpagesize());
	if(argc <  3)
	{
		printf("Usage : <executable> <library 1> <library 2>\n");
		exit(1);
	}
	
	//opening libraries from the arguments
	char lib1_path[30];
	sprintf(lib1_path,"./%s",argv[1]);
	printf("%s\n", lib1_path);
	handle1 = dlopen(lib1_path,RTLD_LAZY);
	if(!handle1)
	{
		fputs(dlerror(),stderr);
		exit(1);
	}
	sprintf(lib1_path,"./%s",argv[2]);
	printf("%s\n", lib1_path);
	handle2 = dlopen(lib1_path,RTLD_LAZY);
	if(!handle2)
	{
		fputs(dlerror(),stderr);
		exit(1);
	}
	printf("int value of handle2 %d\n", (*(int *)handle2));
	//opening /proc/PID/maps to parse for memory mappings
	char map_path[20];
	sprintf(map_path,"/proc/%d/maps",getpid());
	FILE *in;
	in = fopen(map_path,"r");
	if(in == NULL)
	{
		printf("error in opening mapping\n");
		exit(1);
	}
	char line[200];
	int flag = 0, flag2 = 0;
	char *token;
	//getting memory mappings of both the libraries
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
		if(strstr(line,argv[2]) != NULL)
		{
			printf("line in proc : %s\n",line);
			token = strtok(line," ");
			printf("token : %s|\n", token);
			sscanf(token,"%p-%p",&l2_seg_s[flag2], &l2_seg_e[flag2]);
			printf("read addresses %p - %p\n", l2_seg_s[flag2], l2_seg_e[flag2]);
			flag2++;
		}
	}
	fclose(in);
	sizeA = (int)(l1_seg_e[3] - l1_seg_s[0]);
	sizeB = (int)(l2_seg_e[3] - l2_seg_s[0]);
	srcA = l1_seg_s[0];
	srcB = l2_seg_s[0];
	printf("size of allocation of libA(bytes) : %u\n", sizeA);
	printf("size of allocation of libB(bytes) : %u\n", sizeB);
	printf("\n\n\n");

	//set-up interface functions
	void (*sharedfuncA)(void *) = dlsym(handle1,"set_shared");
	sharedfuncA((void *)A2main);

	void (*change)(void) = dlsym(handle2,"changek");
	change();
	void *k = dlsym(handle2,"k");
	printf("k = %d\n", (*(int *)k));
	void (*callfunc)(void) = dlsym(handle1,"callfunc");
	callfunc();
	printf("k = %d\n", (*(int *)k));
	dlclose(handle2);
	dlclose(handle1);
	munmap(addrB, sizeB);
	return 0;
}
