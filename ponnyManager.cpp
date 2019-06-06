#include "ponnyManager.h"

MutexVariable<Ponny*> ponnyValue;
MutexVariable<int> ponnyMutex(0);
extern int rank,size;

void handleUnlockPonny(){
    auto current_section = getCurrentSection();
    current_section->lock();
    if(current_section->get()->id == 0 && current_section->get()->active){
        if(canTakePonny()){
            ponnyMutex.unlock();
        }
    }
    current_section->unlock();
}
bool canTakePonny(){
    int count = 0;
    auto lamport_vector_section = getLamportSection();
    lamport_vector_section->lock();
    
    auto vec = lamport_vector_section->get();
    auto lamp = vec[rank];
    
    for(int i=0;i<vec.size();++i)
        if(i!=rank)
            if(vec[i]<lamp || (vec[i]==lamp && rank>i))
                count++;
    
    lamport_vector_section->unlock();
    return count < ponnyValue.get()->current;
    
}


void takePonny(){
    auto current_section = getCurrentSection();
    auto lamport_vector_section = getLamportSection();
    auto lamport_vector_global = getLamportGlobal();
    
    current_section->lock();
    lamport_vector_section->set(lamport_vector_global->get());

    auto sec = current_section->get();
    sec->id = 0;
    sec->time = lamport_vector_section->get()[rank];
    sec->active = 1;
    
    current_section->unlock();
    
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

    current_section->lock();
    sec = current_section->get();
    sec->active = 0;
    current_section->unlock();
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

void ponnyHandleMessage(packet_send_t mesg){
    if(mesg.action == 0){
        ponnyValue.lock();
        ponnyValue.get()->current -= mesg.value;
        ponnyValue.unlock();
    }
    if(mesg.action == 2){
        ponnyValue.lock();
        ponnyValue.get()->current += mesg.value;
        ponnyValue.unlock();
    }
    handleUnlockPonny();
}
void ponnyStateChange(){
    handleUnlockPonny();
}
void ponnyInit(int _rank, int _size, int count){

    ponnyValue.set(new Ponny(count));
}