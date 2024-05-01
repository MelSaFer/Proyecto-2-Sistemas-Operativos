#ifndef MEMPARTITION_H
#define MEMPARTITION_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include "thread.h"

struct MEMPARTITION{
    struct THREAD *thread;
};

#endif // MEMPARTITION_H