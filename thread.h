#ifndef THREAD_H
#define THREAD_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <pthread.h>
#include <time.h>

enum State { 
    BLOCKED, 
    RUNNING, 
    FINISHED,
    DEAD};

enum ActionType { 
    allocation,
    deallocate
    };

struct THREAD{
    int pid;
    int size;
    int time;
    enum State state;
    // enum ActionType actionType;
    //time_t start;
    //time_t end;
};


#endif // THREAD_H