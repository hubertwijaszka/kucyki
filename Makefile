all: main

main: kucyki.o connection.o ponnyManager.o shipManager.o
	mpic++ out/kucyki.o out/connection.o out/ponnyManager.o out/shipManager.o -o out/main.o

kucyki.o: kucyki.cpp kucyki.h
	mpic++ kucyki.cpp -c -Wall -pthread -o out/kucyki.o

connection.o: connection.cpp connection.h
	mpic++ connection.cpp -c -Wall -o out/connection.o

ponnyManager.o: ponnyManager.cpp ponnyManager.h
	mpic++ ponnyManager.cpp -c -Wall -o out/ponnyManager.o

shipManager.o: shipManager.cpp shipManager.h
	mpic++ shipManager.cpp -c -Wall -o out/shipManager.o

run: kucyki
	clear
	mpirun -np 4 kucyki

clear: 
	rm -r out/*.o