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
bool canTakePonny(){
    int count = 0;
    lamport_vector_section.lock();
    auto vec = lamport_vector_section.get();
    auto lamp = vec[rank];
    for(int i=0;i<vec.size();++i)
        if(i!=rank)
            if(vec[i]<lamp || (vec[i]==lamp && rank>i))
                count++;
    return count < ponny_mutex.get()->current;
    
}

MutexVariable<int> ponnyMutex(0);
void handleUnlockPonny(){
    current_section.lock();
    if(current_section.get()->id == 0 && current_section.get()->active){
        if(canTakePonny())
            ponnyMutex.unlock();
    }
    current_section.unlock();
}

void takePonny(){
    current_section.lock();
    lamport_vector_section.set(lamport_vector_global.get());

    auto sec = current_section.get();
    sec->id = 0;
    sec->time = lamport_vector_section.get()[rank];
    sec->active = 1;
    
    current_section.unlock();
    
    if(canTakePonny()){
        packet_send_t mess;
        mess.section = 0;
        mess.action = 0; //Take
        mess.value = 1;
        for(int i=0;i<size;++i){
            if(i!=rank)
                sendMessage(mess,i,GET_RESOURCE);
        }
    }else{
        packet_send_t mess;
        mess.section = 0;
        mess.action = 3; //Ask
        mess.value = 0;
        for(int i=0;i<size;++i){
            if(i!=rank)
                sendMessage(mess,i,GET_TIME);
        }
        ponnyMutex.lock();
        ponnyMutex.getLock();
        mess.action = 0;
        mess.value = 1;
        for(int i=0;i<size;++i){
            if(i!=rank)
                sendMessage(mess,i,GET_RESOURCE);
        }

    }

    current_section.lock();
    sec = current_section.get();
    sec->active = 0;
    current_section.unlock();
}
void returnPonny(){
    packet_send_t mess;
    mess.section = 0;
    mess.action = 2; //Return
    mess.value = 1;
    for(int i=0;i<size;++i){
        if(i!=rank)
            sendMessage(mess,i,GET_RESOURCE);
    }
}
void mainLoop(){
    // if(rank != 0){
    //     sleep(5);
    // }else{
    //     sleep(1);
    //     packet_send_t mess;
        
    //     for(int i=1;i<size;++i){
    //         printf("%d %d %d\n",i, size, rank);
    //         mess.section = i;
    //         mess.action = i;
    //         mess.value = i;
    //         sendMessage(mess, i, i);
    //     }
    // }
    while(true){
        takePonny();
        printf("%d ponny taken\n",rank);
        sleep(2);
        printf("%d return ponny\n", rank);
        returnPonny();
    }
    //Take ponny
    //Take ship
    //Wait 
    //Do something
    //Come back
    //Give back ponny
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
    

void*  comFunc(void *arg){
    while(1){
        packet_send_t r_mess;
        

        auto mess = recv_message();
        printf("Rank %d From %d\n", rank, mess.first.MPI_SOURCE);
        
        auto status = mess.first;
        auto mesg = mess.second;
        lamport_vector_global.get()[rank] = lamport_vector_global.get()[rank];

        
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
             continue;
            lamport_vector_global.lock();
            auto vec1 = lamport_vector_global.get();
            vec1[status.MPI_SOURCE] = mesg.lamport_clock;
            lamport_vector_global.setNonLock(vec1);
            lamport_vector_global.unlock();

            lamport_vector_section.lock();
            auto vec2 = lamport_vector_section.get();
            vec2[status.MPI_SOURCE] = mesg.lamport_clock;
            lamport_vector_section.setNonLock(vec2);
            lamport_vector_section.unlock();
            
            if(mesg.section == 0){ //Ponny
                if(mesg.action == 0){
                    ponny_mutex.lock();
                    ponny_mutex.get()->current -= mesg.value;
                    ponny_mutex.unlock();
                }
                if(mesg.action == 2){
                    ponny_mutex.lock();
                    ponny_mutex.get()->current += mesg.value;
                    ponny_mutex.unlock();
                }
                handleUnlockPonny();
            }
         }
        // }
    }
    // std::cout << "NEW\n";
    // Messeges buf;
    // MPI_Status status;
    // while(true){
    //     MPI_Recv(&buf, 1,MPI_INT, MPI_ANY_SOURCE, MPI_ANY_TAG, MPI_COMM_WORLD, &status);
    //     mpi_mutex.set(buf.send.lamport_clock+1); //TODO check 
        
    //     if(status.MPI_TAG == GIVE_YOUR_STATE){
    //        Messeges ms;
    //        current_section.lock();
    //        auto sec = current_section.get();
    //        if(sec == nullptr || sec->active == false){
    //            ms.response.section = -1;
    //            ms.response.earliest = mpi_mutex.get();
    //        }else
    //        {
    //            ms.response.section = sec->id;
    //            ms.response.earliest = sec->time;
    //        }
    //        sendMessage(ms, PACKET_RESPONSE_T, status.MPI_SOURCE, STATE_RESPONSE);
    //        current_section.unlock();
    //     }
    //     if(status.MPI_TAG == CHANGE_STATE){
    //         handleChangeState(buf.send, status.MPI_SOURCE);
    //     }
    //     if(status.MPI_TAG == STATE_RESPONSE){
    //         handleStateResponse(buf.response, status.MPI_SOURCE);
    //     }
    // }



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

    create_mpi_types();
    
    
    pthread_create( &threadCom, NULL, comFunc, 0);

    auto tmp1 = std::vector<int>(size);
    lamport_vector_section.set(tmp1);

    auto tmp2 = std::vector<int>(size);
    lamport_vector_global.set(tmp2);

    ponny_mutex.set(new Ponny(1));

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