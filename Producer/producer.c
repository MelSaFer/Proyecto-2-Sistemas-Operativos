/*
Instituto Tecnológico de Costa Rica
Escuela de Ingeniería en Computación
Curso: Principios de Sistemas Operativos
Profesor: Erika Marín Schumman
Proyecto 2: Simulación de asignación de memoria
Estudiantes:
    - Salas Fernández Melany - 2021121147
    - Solano Espinoza Moisés - 2021144322
    - Zelaya Coto Fiorella - 2021453615
*/

#include <stdio.h>
#include <pthread.h>
#include <stdlib.h>
#include <unistd.h>
#include <stdbool.h>
#include <string.h>
#include <semaphore.h>
#include <time.h>
#include "../sharedMem.h"
#include "../thread.h"
#include "../memPartition.h"

#define MIN_TIME 20
#define MAX_TIME 60
#define MIN_SIZE 1
#define MAX_SIZE 10
#define PROCESS_SHARED_MEMORY "shared_mem"
#define SHARED_MEMORY_ID 1

struct SHAREDMEM* sharedControlMemoryPointer;

int threadsQuantity = 0;

int generateRandomNumber(int min, int max) {
    return min + rand() % (max + 1 - min);
}

void *allocateProcess(void *arg) {
    struct THREAD *thread = (struct THREAD *)arg;
    // printf("PID: %d\n", thread->pid);
    // printf("Process with size %d allocated\n", thread->size);
    // printf("Process with time %d\n", thread->time);
    // printf("Process state: %d\n", thread->state);

    return NULL;
}

void createThread() {
    // Create the thread data
    struct THREAD *data = malloc(sizeof(struct THREAD));
    if (data == NULL) {
        printf("Error\n");
        return;
    }

    // Fill the process data
    data->size = generateRandomNumber(MIN_SIZE, MAX_SIZE);
    data->time = generateRandomNumber(MIN_TIME, MAX_TIME);
    data->state = BLOCKED;
    data->pid = threadsQuantity;

    // Create the thread
    pthread_t thread;
    if (pthread_create(&thread, NULL, allocateProcess, (void *)data) != 0) {
        printf("Error al crear el thread\n");
        free(data);
        return;
    }

    // Increase the PID
    threadsQuantity++;

    pthread_join(thread, NULL);
}

void createThreads(int quantity) {
    for (int i = 0; i < quantity; i++) {
        createThread();
    }
}

int main() {
    // Seed for random numbers
    srand(time(NULL));

    // Create the process
    createThreads(5);

    // Create key for shared control memory
    key_t sharedControlMemoryKey = ftok(PROCESS_SHARED_MEMORY, SHARED_MEMORY_ID);
    printf("\nShared control memory key: %d\n", sharedControlMemoryKey);
    // Validate creation of key
    if (sharedControlMemoryKey == -1) {
        perror("ftok");
        exit(1);
    }
    // Get shared control memory
    int sharedControlMemoryId = shmget(sharedControlMemoryKey, sizeof(struct SHAREDMEM), 0666);
    // Validate shared control memory
    if (sharedControlMemoryId < 0) {
        perror("shmget");
        exit(1);
    }
    printf("\nShared control memory segment opened with id %d\n", sharedControlMemoryId);
    // Attach shared control memory
    sharedControlMemoryPointer = (struct SHAREDMEM *) shmat(sharedControlMemoryId, NULL, 0);
    // Validate shared control memory pointer
    if (sharedControlMemoryPointer == (void *) -1) {
        perror("shmat");
        exit(1);
    }
    printf("\nShared control memory attached\n");

    // Verify that sharedControlMemoryPointer is not NULL before accessing it
    if (sharedControlMemoryPointer != NULL) {
        // Print the content of sharedControlMemoryPointer properly
        printf("Shared control memory content: %p\n", (void *)sharedControlMemoryPointer);
    }
    sharedControlMemoryPointer->lines = 5;
    // printf("lines: %d\n", sharedControlMemoryPointer->lines);

    return 0;
}