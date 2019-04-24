all: bank
test: test1.c test1.h
	mpicc test1.c -o test.out
bank: main.o init.o
	mpicc main.o init.o -o bank

init.o: init.c 
	mpicc init.c -c -Wall

main.o: main.c main.h
	mpicc main.c -c -Wall

clear: 
	rm *.o bank
