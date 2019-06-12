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

std::vector<MutexVariable<Ship*>*> ships_vector;

MutexVariable<Section*> current_section;

MutexVariable<std::vector<int>> lamport_vector_section;
MutexVariable<std::vector<int>> lamport_vector_global;
int rank, size;
std::vector<int> processes_weights; //TODO move to shipManager


void check_thread_support(int provided);
void initialize_processes_weights(int size);
void inicjuj(int argc, char **argv);
void mainLoop();
int main(int argc, char **argv)
{
    
    /* Tworzenie wątków, inicjalizacja itp */
    inicjuj(argc,argv);
    mainLoop();
    MPI_Finalize();
    
}

void mainLoop(){

    while(true){
        printf("Rank %d New iteration\n", rank);
        sleep(1);
        takePonny();
        printf("Rank %d ponny taken\n",rank);
        sleep(1);
        
        printf("Rank %d Taking ship\n", rank);
        while(!takeRandomShip());
        printf("Rank %d Ship taken\n",rank);
        sleep(5);
        printf("Rank %d Taking ship to home\n", rank);
        takeBackShip();
        printf("Rank %d Back\n",rank);
        sleep(1);
        returnPonny();
        printf("Rank %d return ponny\n", rank);
        sleep(5);
    }
    //Take ponny
    //Take ship
    //Wait 
    //Do something
    //Come back
    //Give back ponny
}  

void*  comFunc(void *arg){
    while(1){
        packet_send_t r_mess;
        

        auto mess = recv_message();
       // printf("Rank %d From %d TAG %d\n", rank, mess.first.MPI_SOURCE, mess.first.MPI_TAG);
        
        auto status = mess.first;
        auto mesg = mess.second;
       // printf("New message, rank: %d, tag: %d, source: %d, section %d, action: %d, value: %d\n", rank, status.MPI_TAG, status.MPI_SOURCE, mesg.section, mesg.action, mesg.value);
        lamport_vector_global.lock();
        auto vec = lamport_vector_global.get();
        vec[rank] = getMpiMutex()->get();
        lamport_vector_global.setNonLock(vec);
        lamport_vector_global.unlock();

        
         if(status.MPI_TAG == GET_TIME){
                current_section.lock();
                auto sec = current_section.get();
                if(sec->id == mesg.section && sec->active){
                    r_mess.section = mesg.section;
                    r_mess.action = 4; // Response
                    r_mess.value = sec->time;
                    sendMessage(r_mess, status.MPI_SOURCE, RESPONSE_TIME);
                }else{
                    r_mess.section = mesg.section;
                    r_mess.action = 4; // Response
                    r_mess.value = getMpiMutex()->get();
                    sendMessage(r_mess, status.MPI_SOURCE, RESPONSE_TIME);
                }
                current_section.unlock();
         } else if(status.MPI_TAG == GET_RESOURCE){
             
            lamport_vector_global.lock();
            lamport_vector_section.lock();
            auto vec1 = lamport_vector_global.get();
            vec1[status.MPI_SOURCE] = mesg.lamport_clock;
            lamport_vector_global.setNonLock(vec1);
            lamport_vector_global.unlock();


            
            auto vec2 = lamport_vector_section.get();
            vec2[status.MPI_SOURCE] = mesg.lamport_clock;
            lamport_vector_section.setNonLock(vec2);
            lamport_vector_section.unlock();

           
            if(mesg.section == 0){ //Ponny
                ponnyHandleMessage(mesg);
            }else{
                shipHandleMessage(mesg);
            }
         }else if(status.MPI_TAG == RESPONSE_TIME){
            auto current_section = getCurrentSection();
            current_section->lock();
            lamport_vector_section.lock();
            if(current_section->get()->id == mesg.section && current_section->get()->active)
            {
            
                auto tmp = lamport_vector_section.get();
                tmp[status.MPI_SOURCE] = mesg.value;
                lamport_vector_section.setNonLock(tmp);
            }
            current_section->unlock();
            lamport_vector_section.unlock();
            shipStateChange();
            ponnyStateChange();
         }
        
    }




}
void inicjuj(int argc, char **argv)
{
    int provided;
    /*
        Argv:
        n weights of processes n=MPI_SIZE
        one number m. Number of ships
        m numbers of capacity of ships;
    */
   
   
  
    
    MPI_Init_thread(&argc, &argv,MPI_THREAD_MULTIPLE, &provided);
    check_thread_support(provided);

    MPI_Comm_rank(MPI_COMM_WORLD, &rank); 
    MPI_Comm_size(MPI_COMM_WORLD, &size);
    srand(rank);

     processes_weights = std::vector<int>(size);
    int ponnies = atoi(argv[1]);
    for(int i=0;i<size;++i)
        processes_weights[i] = atoi(argv[i+2]);
 
    int num_ships = atoi(argv[size+2]);

    std::vector<int> cap(num_ships);
    for(int i=0;i<num_ships;++i)
        cap[i] = atoi(argv[i+size+3]);

     shipInit(cap);

    //delayStack = g_queue_new();
    //initialize_processes_weights(size);

    create_mpi_types();
    
    
    pthread_create( &threadCom, NULL, comFunc, 0);

    auto tmp1 = std::vector<int>(size);
    lamport_vector_section.set(tmp1);

    auto tmp2 = std::vector<int>(size);
    lamport_vector_global.set(tmp2);

    ponnyInit(rank, size, ponnies);

    auto sec = new Section();
    sec->active = 0;
    current_section.set(sec);

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

MutexVariable<Section*>* getCurrentSection(){
    return &current_section;
}
MutexVariable<std::vector<int>>* getLamportSection(){
    return &lamport_vector_section;
}
MutexVariable<std::vector<int>>* getLamportGlobal(){
    return &lamport_vector_global;
} 