all: memory

memory: memory.o
	gcc -g memory.o -o memory

memory.o: memory.h memory.c
	gcc -g memory.c -c

clean:
	rm -rf *.o memory