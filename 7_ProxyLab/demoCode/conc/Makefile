CC = gcc
CFLAGS = -g -Wall
LDFLAGS = -lpthread

all: hello

csapp.o: csapp.c csapp.h
	$(CC) $(CFLAGS) -c csapp.c

hello.o: hello.c csapp.h
	$(CC) $(CFLAGS) -c hello.c


hello: hello.o csapp.o
	$(CC) $(CFLAGS) csapp.o hello.o -o hello $(LDFLAGS)


clean:
	rm -f hostinfo