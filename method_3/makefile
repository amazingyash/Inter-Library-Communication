mmap: foo.c mylib.h mylib.c mylib2.h mylib2.c
	gcc -shared -o libmy.so -fPIC mylib.c
	gcc -shared -o libmy2.so -fPIC mylib2.c
	gcc -g -o foo foo.c -ldl -lrt

clean:
	rm libmy2.so libmy.so foo