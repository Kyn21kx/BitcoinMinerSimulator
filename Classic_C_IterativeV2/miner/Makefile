all: miner

miner: main.c siggen.o transaction.o list.o block.o
	gcc -g -pthread -o miner main.c siggen.o block.o transaction.o list.o -lm

list.o: list.c list.h
	gcc -g -c list.c -o list.o

block.o: block.c block.h
	gcc -g -c block.c -o block.o

siggen.o: siggen.c siggen.h
	gcc -g -c siggen.c -o siggen.o

transaction.o: transaction.c transaction.h
	gcc -g -c transaction.c -o transaction.o

clean:
	rm *.o miner
