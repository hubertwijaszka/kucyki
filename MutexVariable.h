#pragma once
#ifndef MUTEXVARIABLE_H
#define MUTEXVARIABLE_H
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
        mutex.unlock();
    }

    T get(){
        return value;
    }
    void setNonLock(T value){
        this->value = value;
    }
    void set(T value){
        lock();
        this->value = value;
        unlock();
    }

};

#endif