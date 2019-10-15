CC = gcc
CFLAGS = -g -Wall
LDFLAGS = -lpthread

all: hostinfo hostinfo_i echoclient echoserveri hello

csapp.o: csapp.c csapp.h
	$(CC) $(CFLAGS) -c csapp.c

hostinfo.o: hostinfo.c csapp.h 
	$(CC) $(CFLAGS) -c hostinfo.c

hostinfo_i.o: hostinfo_i.c csapp.h
	$(CC) $(CFLAGS) -c hostinfo_i.c

echo.o: echo.c csapp.h
	$(CC) $(CFLAGS) -c echo.c

echoserveri.o: echoserveri.c csapp.h  
	$(CC) $(CFLAGS) -c echoserveri.c

echoclient.o: echoclient.c csapp.h
	$(CC) $(CFLAGS) -c echoclient.c

hello.o: hello.c csapp.h
	$(CC) $(CFLAGS) -c hello.c


hello: hello.o csapp.o
	$(CC) $(CFLAGS) csapp.o hello.o -o hello $(LDFLAGS)

echoclient: csapp.o echoclient.o
	$(CC) $(CFLAGS) csapp.o echoclient.o -o echoclient $(LDFLAGS)

echoserveri: csapp.o echoserveri.o echo.o
	$(CC) $(CFLAGS) csapp.o echoserveri.o echo.o -o echoserveri $(LDFLAGS)

hostinfo: csapp.o hostinfo.o
	$(CC) $(CFLAGS) csapp.o hostinfo.o -o hostinfo $(LDFLAGS)

hostinfo_i: csapp.o hostinfo_i.o
	$(CC) $(CFLAGS) csapp.o hostinfo_i.o -o hostinfo_i $(LDFLAGS)



clean:
	rm -f hostinfo