all: miner

miner: main.c siggen.o transaction.o List.h List.c Definitions.h
	gcc -g -pthread -o miner main.c Definitions.h siggen.o transaction.o List.c -lm

test: List.h List.c
	gcc -g -o test List.c

siggen.o: siggen.c siggen.h
	gcc -g -c -o siggen.o siggen.c 

transaction.o: transaction.c transaction.h
	gcc -g -c -o transaction.o transaction.c

clean:
	rm miner test *.o
