#ifndef SINGLETON_H
#define SINGLETON_H
#include "myprint.h"



// class Singleton{

// protected:
//     Singleton() = default;
//     Singleton(const Singleton&) = delete;
//     Singleton& operator=(const Singleton&) = delete;

// public:
//     static Singleton& getSingleton(){
//         static Singleton singleton;
//         print("get singleton", &singleton);
//     }
//     ~Singleton(){
//         print("singleton destruct");
//     }

// };

template<typename T>
class Singleton{

protected:
    Singleton() = default;
    Singleton(const Singleton<T>&) = delete;
    Singleton& operator=(const Singleton<T>&) = delete;

public:
    static T& getSingleton(){
        static T singleton;
        //print("get singleton", &singleton);
        return singleton;
    }

};

#endif // SINGLETON_H
