all: virtualmemory

virtualmemory: virtualmemory.o
	gcc -g virtualmemory.o -pthread -o virtualmemory

virtualmemory.o: virtualmemory.h virtualmemory.c
	gcc -g virtualmemory.c -c

clean:
	rm -rf *.o virtualmemory
