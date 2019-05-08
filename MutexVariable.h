#include <mutex>

template<class T> 
class MutexVariable{
    T value;
    std::mutex mutex;
public:
    MutexVariable(T value){
        this->value = value;
    }
    MutexVariable(){}
    T getLock(){
        T res;

        lock();
        res = value;
        unlock();
        return res;
    }
    void lock(){
        mutex.lock();
    }
    void unlock(){
        mutex.lock();
    }

    T get(){
        return value;
    }

    void set(T value){
        lock();
        this->value = value;
        unlock();
    }

};