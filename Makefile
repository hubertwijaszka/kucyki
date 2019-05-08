all: kucyki
test: test1.c test1.h
	mpicc test1.c -o test.out
kucyki: kucyki.cpp kucyki.h
	mpic++ kucyki.cpp -o kucyki -g

init.o: init.c 
	mpicc init.c -c -Wall

main.o: main.c main.h
	mpicc main.c -c -Wall

run: kucyki
	clear
	mpirun -np 4 kucyki

clear: 
	rm *.o kucyki
