cliente: sdstore.c
	gcc -Wall -g -c sdstore.c
	gcc -Wall -g -c utils.c
	gcc utils.o sdstore.o -o sdstore

server: sdstored.c
	gcc -Wall -g -c sdstored.c
	gcc -Wall -g -c utils.c
	gcc utils.o sdstored.o -o sdstored

clean:
	rm auth out_* in_*
