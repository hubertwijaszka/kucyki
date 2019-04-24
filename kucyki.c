// #include <iostream>
// #include <algorithm>

#include <mpi.h>
#include <stdlib.h>
#include <stdio.h>
#include <pthread.h>
#include <semaphore.h>
#include <unistd.h>
#include <string.h>

#define ROOT 0

#define FINISH 1
#define APP_MSG 2
#define GIVE_YOUR_STATE 3
#define MY_STATE_IS 4
#define ACC 5

//pthread_t threadCom, threadM;
void inicjuj(int *argc, char ***argv);
void check_thread_support(int provided);
int main(int argc, char **argv)
{
    /* Tworzenie wątków, inicjalizacja itp */

    inicjuj(&argc,&argv);
}

void inicjuj(int *argc, char ***argv)
{
    int provided;
    //delayStack = g_queue_new();
    MPI_Init_thread(argc, argv,MPI_THREAD_MULTIPLE, &provided);
    check_thread_support(provided);


    /* Stworzenie typu */
    /* Poniższe (aż do MPI_Type_commit) potrzebne tylko, jeżeli
       brzydzimy się czymś w rodzaju MPI_Send(&typ, sizeof(pakiet_t), MPI_BYTE....
    */
    /* sklejone z stackoverflow */
    // const int nitems=FIELDNO; // Struktura ma FIELDNO elementów - przy dodaniu pola zwiększ FIELDNO w main.h !
    // int       blocklengths[FIELDNO] = {1,1,1,1}; /* tu zwiększyć na [4] = {1,1,1,1} gdy dodamy nowe pole */
    // MPI_Datatype typy[FIELDNO] = {MPI_INT, MPI_INT,MPI_INT,MPI_INT}; /* tu dodać typ nowego pola (np MPI_BYTE, MPI_INT) */
    // MPI_Aint     offsets[FIELDNO];

    // offsets[0] = offsetof(packet_t, ts);
    // offsets[1] = offsetof(packet_t, kasa);
    // offsets[2] = offsetof(packet_t, dst);
    // offsets[3] = offsetof(packet_t, src);
    // /* tutaj dodać offset nowego pola (offsets[2] = ... */

    // MPI_Type_create_struct(nitems, blocklengths, offsets, typy, &MPI_PAKIET_T);
    // MPI_Type_commit(&MPI_PAKIET_T);

    int rank,size;
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    
    MPI_Comm_size(MPI_COMM_WORLD, &size);
    printf("Rank %d\n",size);
    srand(rank);
    int weight = rand()%100;
    printf("%d %d\n",rank, weight);
    for(int i=0;i<size;++i){
        if(i!=rank)
        MPI_Send(&weight,1,MPI_INT,i,1,MPI_COMM_WORLD);
    }
    MPI_Bcast(&weight, 1, MPI_INT, 0, MPI_COMM_WORLD);
    MPI_Status status;
    for(int i=1;i<size;++i){
        int buf;
        MPI_Recv(&buf, 1,MPI_INT, MPI_ANY_SOURCE, MPI_ANY_TAG, MPI_COMM_WORLD, &status);
        printf("%d %d\n",rank, buf);
    }
    // pthread_create( &threadCom, NULL, comFunc, 0);
    // //pthread_create( &threadDelay, NULL, delayFunc, 0);
    // if (rank==ROOT) {
	// pthread_create( &threadM, NULL, monitorFunc, 0);
    // } 
}
void check_thread_support(int provided)
{
    printf("THREAD SUPPORT: %d\n", provided);
    switch (provided) {
        case MPI_THREAD_SINGLE: 
            printf("Brak wsparcia dla wątków, kończę\n");
            /* Nie ma co, trzeba wychodzić */
	    fprintf(stderr, "Brak wystarczającego wsparcia dla wątków - wychodzę!\n");
	    MPI_Finalize();
	    exit(-1);
	    break;
        case MPI_THREAD_FUNNELED: 
            printf("tylko te wątki, ktore wykonaly mpi_init_thread mogą wykonać wołania do biblioteki mpi\n");
	    break;
        case MPI_THREAD_SERIALIZED: 
            /* Potrzebne zamki wokół wywołań biblioteki MPI */
            printf("tylko jeden watek naraz może wykonać wołania do biblioteki MPI\n");
	    break;
        case MPI_THREAD_MULTIPLE: printf("Pełne wsparcie dla wątków\n");
	    break;
        default: printf("Nikt nic nie wie\n");
    }
}