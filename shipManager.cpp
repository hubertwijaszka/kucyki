#include "shipManager.h"


std::vector<int> cap;
extern std::vector<int> processes_weights;
std::vector<std::vector<packet_send_t>> que;
std::vector<MutexVariable<int>*> shipValue;
MutexVariable<int> shipMutex;
MutexVariable<int> shipBackMutex;
MutexVariable<int> shipSailMutex;
MutexVariable<int> anyShipMutex;
int selectedSection = -1;
extern int rank,size;

int canTakeShip(int index){
    int count = processes_weights[rank];
    auto lamport_vector_section = getLamportSection();
    lamport_vector_section->lock();
    
    auto vec = lamport_vector_section->get();
    auto lamp = vec[rank];
    
    for(int i=0;i<vec.size();++i)
        if(i!=rank)
            if(vec[i]<lamp || (vec[i]==lamp && rank>i))
                count += processes_weights[i];
    
    lamport_vector_section->unlock();
    
    if(count <= shipValue[index]->get())
        return 1;
    if(count == processes_weights[rank] && shipValue[index]->get()>0)
        return 2;
    return 0;
}
bool shipAvailable(){


    for(auto it : shipValue)
        it->lock();
    bool res = false;
    int i = 0;
    for(auto it : shipValue){
        res |= (it->get()>0 && cap[i]>= processes_weights[rank]);
        i += 1;
    }
    for(auto it : shipValue)
        it->unlock();
    return res;
}
void shipStateChange(){

    if(shipAvailable())
        anyShipMutex.unlock();
    
    auto current_section = getCurrentSection();
    current_section->lock();
    
    auto section = selectedSection;
    if(section>0){
        auto tmp = canTakeShip(section-1);
        if(tmp != 0)
            shipMutex.unlock();
    }
    current_section->unlock();

}
bool takeRandomShip(){
    auto current_section = getCurrentSection();
    auto lamport_vector_section = getLamportSection();
    auto lamport_vector_global = getLamportGlobal();

   

    int section = rand()%cap.size()+1;
    while(cap[section-1]<processes_weights[rank]){
        section = rand()%cap.size()+1;
    }
    current_section->lock();
   
    lamport_vector_section->set(lamport_vector_global->getLock());

    auto sec = current_section->get();
    sec->id = section;
    sec->time = lamport_vector_section->get()[rank];
    sec->active = 1;
    selectedSection = section;
    current_section->unlock();
    
  
    if(!shipAvailable())
    {
        anyShipMutex.lock();
        anyShipMutex.getLock();
    }
    
    if(cap[section-1]< processes_weights[rank])
        return false;
    packet_send_t mess;

    int canTake = canTakeShip(section-1);
    
    if(canTake == 0){
        
        mess.section = section;
        mess.action = 3; //Ask
        mess.value = 0;
        for(int i=0;i<size;++i){
            if(i!=rank)
                sendMessage(mess,i,GET_TIME);
        }
        shipMutex.lock();
        shipMutex.getLock();
        canTake = canTakeShip(section-1);
       
    }

    if(canTake == 1){
        mess.section = section;
        mess.action = 0; //Take
        mess.value = processes_weights[rank];
        for(int i=0;i<size;++i){
            if(i!=rank)
                sendMessage(mess,i,GET_RESOURCE);
        }
    }
   
    if(canTake == 2){ //First one but cannot take place
        
        mess.section = section;
        mess.action = 6; //Sail
        mess.value = 2*shipValue[section-1]->get();
        for(int i=0;i<size;++i){
            if(i!=rank)
                sendMessage(mess,i,GET_RESOURCE);
        }       
    }

    shipValue[section-1]->set(shipValue[section-1]->get()-mess.value);

  
    if(canTake == 2){
        sec = current_section->get();
        sec->active = 0;
        current_section->unlock();
        return false;
    }

    sec = current_section->get();
    sec->active = 0;
    current_section->unlock();

    if(shipValue[section-1]->get()){
        shipSailMutex.lock();
        shipSailMutex.getLock();
    }else{
        mess.section = section;
        mess.action = 6; //Sail
        mess.value = 0;
        for(int i=0;i<size;++i){
            if(i!=rank)
                sendMessage(mess,i,GET_RESOURCE);
        }  
    }
    printf("Rank %d Id of taken ship %d\n",rank, selectedSection-1);

    return true;
}

void takeBackShip(){

    auto current_section = getCurrentSection();
    packet_send_t mess;
    mess.section = selectedSection;
    mess.action = 5; //return back ship
    mess.value = processes_weights[rank];
    for(int i=0;i<size;++i){
        if(i!=rank)
            sendMessage(mess,i,GET_RESOURCE);
    }

    auto ship = shipValue[mess.section-1];
    ship->set(ship->get()-mess.value);
    if(ship->get() <= -cap[selectedSection-1]){
         ship->set(ship->get()+2*cap[selectedSection-1]);
        for(auto it : que[selectedSection-1]){
                ship->set(ship->get()-it.value);
        }
        que[selectedSection-1].clear();
    }else{
        shipBackMutex.lock();
        shipBackMutex.getLock();
    }

    selectedSection = -1;
}
void shipHandleMessage(packet_send_t mesg){
    auto ship = shipValue[mesg.section-1];
    auto current_section = getCurrentSection();
   
    if(mesg.action == 0){
        if(ship->get() <= 0){
            que[mesg.section-1].push_back(mesg);
        }else{
            ship->set(ship->get()-mesg.value);
        }
    }
    if(mesg.action == 6){ //Sail
        ship->set(ship->get()-mesg.value);
        if(selectedSection == mesg.section){
            shipSailMutex.unlock();
        }
    }
    if(mesg.action == 5){ //Return back
        ship->set(ship->get()-mesg.value);
        if(ship->get() <= -cap[mesg.section-1]){
            ship->set(ship->get()+2*cap[mesg.section-1]);
                for(auto it : que[mesg.section-1]){
                    ship->set(ship->get()-it.value);
                }
                que[mesg.section-1].clear();
                shipBackMutex.unlock();
        } 
    }
         
    
    shipStateChange();
}

void shipInit(std::vector<int> capacity){
    cap = capacity;
    que = std::vector<std::vector<packet_send_t>>(cap.size());
    shipValue = std::vector<MutexVariable<int>*>(cap.size());
    for(int i=0;i<cap.size();++i)
        shipValue[i] = new MutexVariable<int>(cap[i]);
}
