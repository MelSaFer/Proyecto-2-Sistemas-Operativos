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

typedef struct {
    pthread_t pid;
    int size;
    int time;
    int state;
    time_t start;
    time_t end;
} THREAD;

// State = 0;

#endif // THREAD_H