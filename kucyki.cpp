// #include <iostream>
// #include <algorithm>
#include "kucyki.h"


#define ROOT 0

#define FINISH 1
#define APP_MSG 2
#define GIVE_YOUR_STATE 3
#define MY_STATE_IS 4
#define CHANGE_STATE 5
#define STATE_RESPONSE 6

pthread_t threadCom, threadM;

void check_thread_support(int provided);
void initialize_processes_weights(int size);
void inicjuj(int *argc, char ***argv);
void mainLoop();
int main(int argc, char **argv)
{
    /* Tworzenie wątków, inicjalizacja itp */
    inicjuj(&argc,&argv);
    mainLoop();
    MPI_Finalize();
}
void mainLoop(){
    //Take ponny
    //Take ship
    //Wait 
    //Do something
    //Come back
    //Give back ponny
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
    
    int blocklengths_response[3] = {1, 1, 1};
    MPI_Datatype types_response[3] = {MPI_INT, MPI_INT, MPI_INT};
    MPI_Aint offsets_response[3];

    offsets_response[0] = offsetof(packet_response_t, section);
    offsets_response[1] = offsetof(packet_response_t, lamport_clock);
    offsets_response[2] = offsetof(packet_response_t, earliest);
   
    MPI_Type_create_struct(3, blocklengths_response, offsets_response, types_response, &MPI_RESPONSE_TYPE);
    MPI_Type_commit(&MPI_RESPONSE_TYPE);

}
void sendMessage(Messeges message, int type,int destination,int tag){
    mpi_mutex.lock();
    int time = mpi_mutex.get();
    time++;
    if(type == PACKET_RESPONSE_T)
    {
        packet_response_t m = message.response;
        m.lamport_clock = time;
        MPI_Send(&m, 1, MPI_RESPONSE_TYPE, destination, tag, MPI_COMM_WORLD);
    }
    if(type == PACKET_SEND_T)
    {
        packet_send_t m = message.send;
        m.lamport_clock = time;
        MPI_Send(&m, 1, MPI_RESPONSE_TYPE, destination, tag, MPI_COMM_WORLD);
    }
    mpi_mutex.set(time);
    mpi_mutex.unlock();
}
void updateVector(MutexVariable<std::vector<int>> &vec, int position, int value, int mode){
    vec.lock();
    auto section_vec = vec.get();
    if(mode == 0)
        section_vec[position] = std::max(value,section_vec[position]);
    else
        section_vec[position] = value;
    vec.set(section_vec);
    vec.unlock();
}
void handleChangeState(packet_send_t msg, int source){
    updateVector(lamport_vector_section, source, msg.lamport_clock, 0);
    updateVector(lamport_vector_global, source, msg.lamport_clock, 0);
   
    if(msg.section == 0){
        ponny_mutex.lock();
        auto ponny = ponny_mutex.get();
        ponny->current += msg.value;
        ponny_mutex.unlock();
    }    
    else{
    
    }
}
    
void handleStateResponse(packet_response_t msg, int source){

    auto sec = current_section.getLock();
    if(sec != nullptr && sec->id == msg.section){
        updateVector(lamport_vector_section, source, msg.section, 1);
    }
}   
void*  comFunc(void *arg){
    std::cout << "NEW\n";
    Messeges buf;
    MPI_Status status;
    while(true){
        MPI_Recv(&buf, 1,MPI_INT, MPI_ANY_SOURCE, MPI_ANY_TAG, MPI_COMM_WORLD, &status);
        mpi_mutex.set(buf.send.lamport_clock+1); //TODO check 
        
        if(status.MPI_TAG == GIVE_YOUR_STATE){
           Messeges ms;
           current_section.lock();
           auto sec = current_section.get();
           if(sec == nullptr || sec->active == false){
               ms.response.section = -1;
               ms.response.earliest = mpi_mutex.get();
           }else
           {
               ms.response.section = sec->id;
               ms.response.earliest = sec->time;
           }
           sendMessage(ms, PACKET_RESPONSE_T, status.MPI_SOURCE, STATE_RESPONSE);
           current_section.unlock();
        }
        if(status.MPI_TAG == CHANGE_STATE){
            handleChangeState(buf.send, status.MPI_SOURCE);
        }
        if(status.MPI_TAG == STATE_RESPONSE){
            handleStateResponse(buf.response, status.MPI_SOURCE);
        }
    }
}
void inicjuj(int *argc, char ***argv)
{
    int provided;
    //delayStack = g_queue_new();
    
    MPI_Init_thread(argc, argv,MPI_THREAD_MULTIPLE, &provided);
    check_thread_support(provided);

    MPI_Comm_rank(MPI_COMM_WORLD, &rank); 
    MPI_Comm_size(MPI_COMM_WORLD, &size);
    srand(rank);
    initialize_processes_weights(size);
    for(auto it : processes_weights){
        std::cout << it << "size\n";
    }
    create_mpi_types();
    
    
    pthread_create( &threadCom, NULL, comFunc, 0);
    // //pthread_create( &threadDelay, NULL, delayFunc, 0);
    // if (rank==ROOT) {
	// pthread_create( &threadM, NULL, monitorFunc, 0);
    // } 
}
void initialize_processes_weights(int size){
    processes_weights = std::vector<int>(size);
    
    int my_weight = rand()%(MAX_WEIGHT)+1;
    processes_weights[rank] = my_weight;
    for(int i=0;i<size;++i){
        if(i!=rank)
        MPI_Send(&my_weight,1,MPI_INT,i,TAG_INITIALIZE,MPI_COMM_WORLD);
    }
    MPI_Status status;
    for(int i=1;i<size;++i){
        int buf;
        MPI_Recv(&buf, 1,MPI_INT, MPI_ANY_SOURCE, TAG_INITIALIZE, MPI_COMM_WORLD, &status);
        processes_weights[status.MPI_SOURCE] = buf;
    }
}
void check_thread_support(int provided)
{
    
    switch (provided) {
        case MPI_THREAD_SINGLE: 
            
            /* Nie ma co, trzeba wychodzić */
            fprintf(stderr, "Brak wystarczającego wsparcia dla wątków - wychodzę!\n");
            MPI_Finalize();
            exit(-1);
            break;
      }
}