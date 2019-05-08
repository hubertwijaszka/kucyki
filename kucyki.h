#include "MutexVariable.h"
#include <mpi.h>
#include <stdlib.h>
#include <stdio.h>
#include <pthread.h>
#include <semaphore.h>
#include <unistd.h>
#include <string.h>
#include <iostream>
#include <vector>

#define MAX_WEIGHT 10
#define ROOT 0

#define TAG_INITIALIZE 1
#define FINISH 1
#define APP_MSG 2
#define GIVE_YOUR_STATE 3
#define MY_STATE_IS 4
#define ACC 5

int rank, size;
std::vector<int> processes_weights;
MPI_Datatype mpi_send_type, mpi_response_type;

struct Ponny{
    int current;
    Ponny(int number){
        current = number;
    }
};
struct Ship{
    int places;
    Ship(int places){
        this->places;
    }
};
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

union Messeges{
    packet_send_t send;
    packet_response_t response;
};

struct Section{
    int id, time, active; //ID - unique number, time-lamport clocks time, -active
};

MutexVariable<void> mpi_mutex(void);
MutexVariable<Ponny*> ponny_mutex;
MutexVariable<int> lamport_mutex(0);
MutexVariable<Section*> current_section;

MutexVariable<std::vector<int>> lamport_vector;
std::vector<MutexVariable<Ship*>> shis_vector;
