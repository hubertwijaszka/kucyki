#pragma once
#ifndef CONNECTION_H
#define CONNECTION_H

#include <mpi.h>
#include "MutexVariable.h"
#include <utility>


#define PACKET_RESPONSE_T 1
#define PACKET_SEND_T 2

#define GET_RESOURCE 3
#define GET_TIME 4
#define RESPONSE_TIME 5

struct packet_send_t {
    int section;
    int lamport_clock;
    int action;
    int value;
    };

// typedef struct {
//     int section;
//     int lamport_clock;
//     int earliest;
// } packet_response_t;

// union Messeges{
//     packet_send_t send;
//     packet_response_t response;
// };

MutexVariable<int>* getMpiMutex();
void create_mpi_types();
void sendMessage(packet_send_t message, int destination,int tag);
std::pair<MPI_Status, packet_send_t> recv_message();


#endif