all: main

main: kucyki.o connection.o
	mpic++ kucyki.o connection.o -o main

kucyki.o: kucyki.cpp kucyki.h
	mpic++ kucyki.cpp -c -Wall

connection.o: connection.cpp connection.h
	mpic++ connection.cpp -c -Wall

run: kucyki
	clear
	mpirun -np 4 kucyki

clear: 
	rm *.o kucyki
