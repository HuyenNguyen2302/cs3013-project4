all: virtualmemory

memory: virtualmemory.o
	gcc -g virtualmemory.o -o virtualmemory

memory.o: virtualmemory.h virtualmemory.c
	gcc -g virtualmemory.c -c

clean:
	rm -rf *.o virtualmemory
