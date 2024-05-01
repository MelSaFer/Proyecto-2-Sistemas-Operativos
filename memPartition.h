#ifndef MEMPARTITION_H
#define MEMPARTITION_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include "thread.h"

typedef struct {
    struct THREAD *thread;
} MEMPARTITION;

// State = 0;

#endif // MEMPARTITION_H