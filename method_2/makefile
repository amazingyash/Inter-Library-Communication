sharedmem: main.c mylib.h mylib.c mylib2.h mylib2.c
	gcc -shared -o libmy.so -fPIC mylib.c
	gcc -shared -o libmy2.so -fPIC mylib2.c
	gcc -g -o main main.c -ldl -lrt

test:
	rm log.txt
	bash script.sh 100
	./avg

clean:
	rm libmy2.so libmy.so main