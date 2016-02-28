all: memory2

memory: memory.o
	gcc -g memory.o -o memory

memory.o: memory.h memory.c
	gcc -g memory.c -c

memory2: memory2.o
	gcc -g memory2.o -o memory2

memory2.o: memory.h memory2.c
	gcc -g memory2.c -c

clean:
	rm -rf *.o memory2