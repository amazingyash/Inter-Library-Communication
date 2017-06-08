#include <stdio.h>
#include <stdlib.h>
#include <dlfcn.h>
#include <string.h>
#include <sys/types.h>
#include <unistd.h>
#include <sys/mman.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <errno.h>

void *handle1, *handle2, *addrA, *addrB;
void *l1_seg_s[3], *l1_seg_e[3];
unsigned int sizeA;
void *l2_seg_s[3], *l2_seg_e[3];
unsigned int sizeB;

unsigned int interface_A_main(char *func_name)
{
	// printf("Inside interace a main\n");
	void *func = dlsym(handle2,func_name);   // gets pointer to requested function in the library 2
	unsigned int func_offset = func - l2_seg_s[0];  // gets its offset from the starting of memory mapping
	// printf("offset returning %u\n", func_offset);
	return func_offset;  // returns the offset
}

unsigned int interface_B_main(char *func_name)
{
	void *func = dlsym(handle1,func_name);
	unsigned int func_offset = func - l1_seg_s[0];
	return func_offset;
}

int main(int argc, char const *argv[])
{
	printf("%d %d\n", getpid(), getpagesize());
	if(argc < 3)
	{
		printf("Usage : <executable> <library 1 name> <library 2 name>\n");
		exit(1);
	}

	//opening libraries from the arguments
	char lib1_path[30];
	sprintf(lib1_path,"./%s",argv[1]);  // path to library
	printf("%s\n", lib1_path);
	handle1 = dlopen(lib1_path,RTLD_LAZY);
	if(!handle1)  // checking for error in linking library
	{
		fputs(dlerror(),stderr);
		exit(1);
	}
	sprintf(lib1_path,"./%s",argv[2]); // path to second library
	printf("%s\n", lib1_path);   //
	handle2 = dlopen(lib1_path,RTLD_LAZY);
	if(!handle2)
	{
		fputs(dlerror(),stderr);
		exit(1);
	}

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
		fgets(line,200,in);  // reads line from the file
		if(strstr(line,argv[1]) != NULL)  // checks if the name of library 1 is in that line of not
		{
			// printf("line in proc : %s\n",line);
			token = strtok(line," "); // breaks the till first space and the first segment is stored in token
			printf("token : %s|\n", token);
			sscanf(token,"%p-%p",&l1_seg_s[flag], &l1_seg_e[flag]);
			printf("read addresses %p - %p\n", l1_seg_s[flag], l1_seg_e[flag]); 
			flag++;
		}
		if(strstr(line,argv[2]) != NULL)
		{
			// printf("line in proc : %s\n",line);
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
	printf("size of allocation of libA(bytes) : %u\n", sizeA);
	printf("size of allocation of libB(bytes) : %u\n", sizeB);
	printf("\n\n\n");

	//setting up interface functions
	void *sharedA = dlsym(handle1,"set_shared_A");
	void (*set_shared_a)(void *, unsigned int) = (void (*)(void *, unsigned int))sharedA;
	set_shared_a((void *)interface_A_main, sizeB);
	void *sharedB = dlsym(handle2,"set_shared_B");
	void (*set_shared_b)(void *, unsigned int) = (void (*)(void *, unsigned int))sharedB;
	set_shared_b((void *)interface_B_main, sizeA);


	for(int i = 0 ; i < 4 ; i++)
	printf("%p - %p\n", l1_seg_s[i], l1_seg_e[i]);
	printf("\n\n\n");
	for(int i = 0 ; i < 4 ; i++)
	printf("%p - %p\n", l2_seg_s[i], l2_seg_e[i]);

	//making new shared memory for library A
	int fdA,fdB;
	fdA = shm_open("/functions_A", O_RDWR | O_CREAT, 0777);
	if(fdA == -1)
	{
		fprintf(stderr, "Open failed:%s\n", strerror(errno));
		return EXIT_FAILURE;
	}
	if(ftruncate(fdA,sizeA) == -1)
	{
		fprintf(stderr, "ftruncate: %s\n", strerror(errno));
		return EXIT_FAILURE;
	}
	//making a new mapping in this address space
	addrA = mmap(NULL,sizeA, PROT_READ | PROT_WRITE | PROT_EXEC, MAP_SHARED, fdA, 0);
	printf("address returned by addrA %p\n", addrA);
	if(addrA == (void *)-1)
	{
		fprintf(stderr, "mmap failed:%s\n", strerror(errno));
		return EXIT_FAILURE;
	}
	//copying the data in the new mapping
	memcpy(addrA, l1_seg_s[0], l1_seg_e[0] - l1_seg_s[0]);
	void *temp;
	printf("libA : Segment1 copied successfully : %p -> %p\n", addrA, addrA + (l1_seg_e[0] - l1_seg_s[0]));
	temp = addrA + (l1_seg_e[0] - l1_seg_s[0]);
	
	// segment 2 skipped because we don't have read access and probably don't need it.

	temp = temp + (l1_seg_e[1] - l1_seg_s[1]);
	memcpy(temp ,l1_seg_s[2] , l1_seg_e[2] - l1_seg_s[2]);
	printf("libA : Segment3 copied successfully : %p -> %p\n", temp, temp + (l1_seg_e[2] - l1_seg_s[2]));
	temp = temp + (l1_seg_e[2] - l1_seg_s[2]);
	memcpy(temp + 1000,l1_seg_s[3] + 1000, l1_seg_e[3] - l1_seg_s[3] - 1000);
	printf("libA : Segment4 copied successfully : %p -> %p\n", temp + 1000, temp + (l1_seg_e[3] - l1_seg_s[3]));
	printf("address of new mapping of library A %p\n", addrA);
	
	//creating a new shared memory for library B
	fdB = shm_open("/functions_B", O_RDWR | O_CREAT, 0777);
	if(fdB == -1)
	{
		fprintf(stderr, "Open failed:%s\n", strerror(errno));
		return EXIT_FAILURE;
	}
	if(ftruncate(fdB,sizeB) == -1)
	{
		fprintf(stderr, "ftruncate: %s\n", strerror(errno));
		return EXIT_FAILURE;
	}
	//making a new mapping in this address space
	addrB = mmap(NULL,sizeB, PROT_READ | PROT_WRITE | PROT_EXEC, MAP_SHARED, fdB, 0);
	// printf("address returned by addrB %p\n", addrB);
	if(addrB == (void *)-1)
	{
		fprintf(stderr, "mmap failed:%s\n", strerror(errno));
		return EXIT_FAILURE;
	}
	// printf("new library B mapping %p to %p\n", addrB, addrB + sizeB);
	// printf("size of segment 4 of library B after mmap %p, %p, %p\n", (l2_seg_e[3] - l2_seg_s[3]), l2_seg_s[3], l2_seg_e[3]);
	//copying the data in the new mapping
	memcpy(addrB, l2_seg_s[0], l2_seg_e[0] - l2_seg_s[0]);
	printf("libB : Segment1 copied successfully : %p -> %p\n", addrB, addrB + (l2_seg_e[0] - l2_seg_s[0]));
	temp = NULL;
	temp = addrB + (l2_seg_e[0] - l2_seg_s[0]);
	
	// segment 2 skipped because we don't have read access

	temp = temp + (l2_seg_e[1] - l2_seg_s[1]);
	memcpy(temp ,l2_seg_s[2] , l2_seg_e[2] - l2_seg_s[2]);
	printf("libB : Segment3 copied successfully : %p -> %p\n", temp, temp + (l2_seg_e[2] - l2_seg_s[2]));
	// printf("value of temp before adding : %p\n", temp);
	temp = temp + (l2_seg_e[2] - l2_seg_s[2]);
	// printf("value of temp after adding : %p\n", temp);
	// printf("size of seg1 and 4 %p %p\n", (l2_seg_e[0] - l2_seg_s[0]), (l2_seg_e[3] - l2_seg_s[3]));
	// printf("libB : Segment4 copied successfully : %p -> %p\n", temp + 1000, temp + (l2_seg_e[3] - l2_seg_s[3] - 1));
	memcpy(temp + 1000,l2_seg_s[3] + 1000, /*l2_seg_e[3] - l2_seg_s[3] - 1000*/0x1000);
	printf("libB : Segment4 copied successfully : %p -> %p\n", temp + 1000, temp + 0x1000);
	printf("address of new mapping of library B %p\n", addrB);

	//syncing the files so that the contents are written back to the memory
	msync(addrA, sizeA, MS_SYNC);
	msync(addrB, sizeB, MS_SYNC);
	// char c;
	// c = getchar();
	// void *func_ptr = dlsym(handle1, "func_A");
	// int (*func_ptr_c)(int) = (int (*)(int))func_ptr;
	// printf("re %d\n",func_ptr_c(5));

	void *func_ptr = dlsym(handle1, "funcA");            // getting the function from library 1 which will call a function in library 2
	void (*func_cat)(void) = (void (*)(void))func_ptr;
	func_cat();
	void *lemon = dlsym(handle2,"lemon");


	void *func_ptr_2 = dlsym(handle2,"funcB");     // getting a function in library 2 which will call a function in library 1
	void (*func_ptr_c_2)(void) = (void (*)(void))func_ptr_2;
	func_ptr_c_2();


	//cleanup
	munmap(addrA,sizeA);
	munmap(addrB,sizeB);
	dlclose(handle1);
	dlclose(handle2);
	shm_unlink("/functions_A");
	shm_unlink("/functions_B");
	return 0;
}