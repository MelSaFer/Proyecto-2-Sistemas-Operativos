#ifndef SHAREDMEM_H
#define SHAREDMEM_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#define MAX_LINES 100

typedef struct {
    int lines;
    struct MEMPARTITION partitions[MAX_LINES];
} SHAREDMEM;

// State = 0;

#endif //SHAREDMEM_H