#include "connection.h"

MPI_Datatype MPI_SEND_TYPE;//, MPI_RESPONSE_TYPE;
MutexVariable<int> mpi_mutex(0);

MutexVariable<int>* getMpiMutex(){
    return &mpi_mutex;
}


void create_mpi_types(){
    int blocklengths_send[4] = {1,1, 1, 1};
    MPI_Datatype types_send[4] = {MPI_INT, MPI_INT, MPI_INT, MPI_INT};
    
    MPI_Aint offsets_send[4];

    offsets_send[0] = offsetof(packet_send_t, section);
    offsets_send[1] = offsetof(packet_send_t, lamport_clock);
    offsets_send[2] = offsetof(packet_send_t, action);
    offsets_send[3] = offsetof(packet_send_t, value);
    MPI_Type_create_struct(4, blocklengths_send, offsets_send, types_send, &MPI_SEND_TYPE);
    MPI_Type_commit(&MPI_SEND_TYPE);

    //-------------------------------------------------------------------------------------------------------
    
    // int blocklengths_response[3] = {1, 1, 1};
    // MPI_Datatype types_response[3] = {MPI_INT, MPI_INT, MPI_INT};
    // MPI_Aint offsets_response[3];

    // offsets_response[0] = offsetof(packet_response_t, section);
    // offsets_response[1] = offsetof(packet_response_t, lamport_clock);
    // offsets_response[2] = offsetof(packet_response_t, earliest);
   
    // MPI_Type_create_struct(3, blocklengths_response, offsets_response, types_response, &MPI_RESPONSE_TYPE);
    // MPI_Type_commit(&MPI_RESPONSE_TYPE);

}
void sendMessage(packet_send_t message, int destination,int tag){
    mpi_mutex.lock();
    int time = mpi_mutex.get();
    time++;

    message.lamport_clock = time;
   // printf("%d %d %d %d \n", message.section, message.lamport_clock, message.value, message.action);
  //  printf("%d %d\n", destination, tag);
    MPI_Send(&message, 1, MPI_SEND_TYPE, destination, tag, MPI_COMM_WORLD);
    mpi_mutex.setNonLock(time);
    mpi_mutex.unlock();

}
std::pair<MPI_Status, packet_send_t> recv_message(){
    packet_send_t buf;
    MPI_Status status;
    MPI_Recv(&buf, 1,MPI_SEND_TYPE, MPI_ANY_SOURCE, MPI_ANY_TAG, MPI_COMM_WORLD, &status);
    mpi_mutex.lock();
    mpi_mutex.setNonLock(std::max(mpi_mutex.get(), buf.lamport_clock)+1);
    mpi_mutex.unlock();
    return std::make_pair(status, buf);
}