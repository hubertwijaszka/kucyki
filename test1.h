
#ifndef MAINH
#define MAINH

#include <mpi.h>
#include <stdlib.h>
#include <stdio.h>
#include <pthread.h>
#include <semaphore.h>
#include <unistd.h>
#include <string.h>

#define MAX_WEIGHT 10
#define ROOT 0

#define TAG_INITIALIZE 1
#define FINISH 1
#define APP_MSG 2
#define GIVE_YOUR_STATE 3
#define MY_STATE_IS 4
#define ACC 5

int rank, size;
int *processes_weights;
MPI_Datatype mpi_send_type, mpi_response_type;


typedef struct {
    int section;
    int lamport_clock;
    int action;
    int value;
    } packet_send_t;
typedef struct {
    int section;
    int lamport_clock;
    int earliest;
} packet_response_t;

void initialize_processes_weights(int size);
void inicjuj(int *argc, char ***argv);
void check_thread_support(int provided);

#endif