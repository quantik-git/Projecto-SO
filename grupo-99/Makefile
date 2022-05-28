CC = gcc
CFLAGS = -Wall -g -c

all: server client

server: bin/sdstored

client: bin/sdstore

bin/sdstored: obj/sdstored.o obj/utils.o obj/comms.o obj/transform.o obj/task.o
	$(CC) obj/utils.o obj/communication.o obj/transform.o obj/task.o obj/sdstored.o -o bin/sdstored

obj/sdstored.o: src/sdstored.c
	$(CC) $(CFLAGS) src/sdstored.c -o obj/sdstored.o

bin/sdstore: obj/sdstore.o obj/utils.o obj/comms.o
	$(CC) obj/utils.o obj/communication.o obj/sdstore.o -o bin/sdstore

obj/sdstore.o: src/sdstore.c
	$(CC) $(CFLAGS) src/sdstore.c -o obj/sdstore.o

obj/transform.o: src/transform.c obj/task.o obj/utils.o
	$(CC) $(CFLAGS) src/transform.c -o obj/transform.o

obj/task.o: src/task.c obj/comms.o
	$(CC) $(CFLAGS) src/task.c -o obj/task.o

obj/comms.o: src/communication.c
	$(CC) $(CFLAGS) src/communication.c -o obj/communication.o

obj/utils.o: src/utils.c
	$(CC) $(CFLAGS) src/utils.c -o obj/utils.o

clean:
	rm obj/* tmp/* bin/{sdstore,sdstored}

debug:
	gdb --args bin/sdstored etc/sdstored.conf bin/sdstore-transformations

test:
	bin/sdstore samples/sample-1.mp3 tmp/sample-1.mp3
	bin/sdstore samples/sample-2.mp3 tmp/sample-2.mp3
