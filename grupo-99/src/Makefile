CC = gcc
CFLAGS = -Wall -g -c

all: client server clean

client: sdstore.c utils comms
	$(CC) $(CFLAGS) sdstore.c
	$(CC) utils.o communication.o sdstore.o -o sdstore

server: sdstored.c utils comms transform task
	$(CC) $(CFLAGS) sdstored.c
	$(CC) utils.o communication.o transform.o task.o sdstored.o -o sdstored

transform: transform.c task utils
	$(CC) $(CFLAGS) transform.c

task: task.c comms
	$(CC) $(CFLAGS) task.c

comms: communication.c
	$(CC) $(CFLAGS) communication.c

utils: utils.c
	$(CC) $(CFLAGS) utils.c

clean:
	rm auth term c2s_* s2c_* *.o term.txt

debug:
	gdb --args ./sdstored ../etc/sdstored.conf ../bin/sdstore-transformations
