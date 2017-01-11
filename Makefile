CC = gcc
CFLAGS = -Wall

main: main.c ratcommands.o
	$(CC) main.c $(CFLAGS) -o ratsg ratcommands.o

ratcommands: ratcommands.c
	$(CC) -c ratcommands.c $(CFLAGS) -o ratcommands.o

test: test.c ratcommands.o
	$(CC) $(CFLAGS) test.c -o test ratcommands.o

run: main
	./ratsg

