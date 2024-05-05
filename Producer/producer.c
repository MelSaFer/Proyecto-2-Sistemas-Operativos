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
#include <string.h>
#include <semaphore.h>
#include <fcntl.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include "../sharedMem.h"
#include "../thread.h"

#define MIN_TIME 20
#define MAX_TIME 60
#define MIN_SIZE 1
#define MAX_SIZE 10
#define SHARED_MEMORY "shared_mem"
#define SHARED_MEMORY_ID 1

struct SHAREDMEM* sharedControlMemoryPointer;
sem_t *sharedMemorySemaphore;

int threadsQuantity = 0;

int generateRandomNumber(int min, int max) {
    return min + rand() % (max - min + 1);
}

void *allocateProcess(void *arg) {
    struct THREAD *thread = (struct THREAD *)arg;
    // sem_wait(sharedMemorySemaphore);  // Esperar por el semáforo antes de modificar la memoria compartida

    // // Lógica de asignación de memoria aquí
    // printf("Thread %d with size %d and time %d started.\n", thread->pid, thread->size, thread->time);

    // sem_post(sharedMemorySemaphore);  // Liberar el semáforo después de modificar
    // sleep(thread->time);  // Simular el tiempo de ejecución

    free(thread);  // Liberar la memoria del hilo
    return NULL;
}

void createThread() {
    struct THREAD *data = malloc(sizeof(struct THREAD));
    if (!data) {
        fprintf(stderr, "Failed to allocate memory for thread data\n");
        return;
    }

    data->size = generateRandomNumber(MIN_SIZE, MAX_SIZE);
    data->time = generateRandomNumber(MIN_TIME, MAX_TIME);
    data->pid = threadsQuantity;

    pthread_t thread;
    if (pthread_create(&thread, NULL, allocateProcess, (void *)data) != 0) {
        fprintf(stderr, "Failed to create thread\n");
        free(data);
    } else {
        pthread_join(thread, NULL);
        ++threadsQuantity;
    }
}

//
void createThreads(int num_threads) {
    for (int i = 0; i < num_threads; i++) {
        createThread();
    }
}

int main() {
    srand(time(NULL));

    //Accesing shared memory
    key_t key = ftok(SHARED_MEMORY, SHARED_MEMORY_ID);
    int shm_id = shmget(key, sizeof(struct SHAREDMEM), 0666);
    sharedControlMemoryPointer = (struct SHAREDMEM*) shmat(shm_id, NULL, 0);

    if (sharedControlMemoryPointer == (void*)-1) {
        perror("shmat failed");
        exit(EXIT_FAILURE);
    }
    //CantLineas en memoria
    printf("Cantidad de lineas en memoria: %d\n", sharedControlMemoryPointer->lines);

    // Open the semaphore
    sharedMemorySemaphore = sem_open("sharedMemorySemaphore", 0);
    if (sharedMemorySemaphore == SEM_FAILED) {
        perror("sem_open failed");
        exit(EXIT_FAILURE);
    }

    int num_threads = 5;  // Suponiendo que queremos crear 5 hilos
    for (int i = 0; i < num_threads; i++) {
        createThread();
    }

    
    sem_close(sharedMemorySemaphore);
    shmdt(sharedControlMemoryPointer);

    return 0;
}
