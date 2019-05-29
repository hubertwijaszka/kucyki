#include "MutexVariable.h"
#include "connection.h"
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


#define TAKE_ACTION 1
#define GIVE_ACTION 2
struct Ponny{
    int current;
    Ponny(int number){
        current = number;
    }
};
struct Ship{
    int places;
    Ship(int places){
        this->places =places;
    }
};


struct Section{
    int id, time, active; //ID - unique number, time-lamport clocks time, -active
};


 //With lamport time
MutexVariable<Ponny*> ponny_mutex;
MutexVariable<Section*> current_section;

MutexVariable<std::vector<int>> lamport_vector_section;
MutexVariable<std::vector<int>> lamport_vector_global;

//export MutexVariable<int> mpi_mutex;
std::vector<MutexVariable<Ship*>*> ships_vector;
