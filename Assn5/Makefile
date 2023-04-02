server: server.c lib
	gcc server.c -o server -I. -L. -lmsocket -lpthread

client: client.c lib
	gcc client.c -o client -I. -L. -lmsocket - lpthread

lib: mysocket.o
	ar rcs libmsocket.a mysocket.o
 
mysocket.o:	mysocket.c mysocket.h
	gcc -c mysocket.c

clean:
	rm -f *.o *.a server client