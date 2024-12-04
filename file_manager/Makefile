CC=gcc
CFLAGS=-Wall -pthread
LDFLAGS=-lz

all: file_manager

file_manager: main.o file_manager.o
	$(CC) $(CFLAGS) -o file_manager main.o file_manager.o $(LDFLAGS)

main.o: main.c file_manager.h
	$(CC) $(CFLAGS) -c main.c

file_manager.o: file_manager.c file_manager.h
	$(CC) $(CFLAGS) -c file_manager.c

clean:
	rm -f *.o file_manager