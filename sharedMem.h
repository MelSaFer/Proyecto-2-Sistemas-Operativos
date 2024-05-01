#ifndef SHAREDMEM_H
#define SHAREDMEM_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
//#include "memPartition.h"
#include "thread.h"

#define MAX_LINES 100

struct SHAREDMEM{
    int lines;
    //struct MEMPARTITION partitions[MAX_LINES];
    struct THREAD partitions[MAX_LINES];
};

// State = 0;

#endif //SHAREDMEM_H