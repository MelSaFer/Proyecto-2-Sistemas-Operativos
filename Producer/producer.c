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
int algorithm = 0;

int generateRandomNumber(int min, int max) {
    return min + rand() % (max - min + 1);
}

//ALGORITMS OF MEM ASSIGNATION
void firstFit(void *arg){

}

void bestFit(void *arg){

}

void worstFit(void *arg){
    struct THREAD *thread = (struct THREAD *)arg;

    int startIndex = 0;
    int counter = 0;
    int maxNum = 0;
    int maxNumIndex = 0;
    bool first = true;

    // Iterate through the array of partitions
    for (int i = 0; i < sharedControlMemoryPointer->lines; i++) {
        // If the partition is emtpy
        if (sharedControlMemoryPointer->partitions[i] == NULL) {
            // Start counting the empty lines
            if (first) {
                startIndex = i;
                first = false;
            }
            counter++;

        } else {
            // Calculates the space left in the partition
            if (counter > maxNum) {
                maxNum = counter;
                maxNumIndex = startIndex;
            }
            counter = 0;
            first = true;
        }

    }

    // If the maxNum is less than the size of the thread, then the thread can't be allocated
    // and the function returns
    if (maxNum < thread->size) {
        return;
    }

    // semaphore wait
    sem_wait(sharedMemorySemaphore);
    // Allocate the thread in the each partition
    int end = maxNumIndex + thread->size;
    for (int i = maxNumIndex; i < end; i++) {
        // Assign the thread to the partition
        sharedControlMemoryPointer->partitions[i] = thread;
    }
    // semaphore post
    sem_post(sharedMemorySemaphore);    
}

//----------------------------------------------------

void registerProcess(void *arg){

}

void deallocateProcess(void *arg) {
    struct THREAD *thread = (struct THREAD *)arg;
    // sem_wait(sharedMemorySemaphore);  // Esperar por el semáforo antes de modificar la memoria compartida

    //
    // printf("Thread %d finished.\n", pid);

    // sem_post(sharedMemorySemaphore);  // Liberar el semáforo después de modificar
    free(thread);  // Liberar la memoria del hilo
    return NULL;
}   

/*----------------------------------------------------
Allocate the process in memory
Entries:
    void *arg: thread data
----------------------------------------------------*/
void *allocateProcess(void *arg) {
    struct THREAD *thread = (struct THREAD *)arg;
    if(algorithm == 0){
        firstFit(thread);
    }else if(algorithm == 1){
        bestFit(thread);
    }else{
        worstFit(thread);
    }

    // sem_wait(sharedMemorySemaphore);  // Esperar por el semáforo antes de modificar la memoria compartida

    // // Lógica de asignación de memoria aquí
    // printf("Thread %d with size %d and time %d started.\n", thread->pid, thread->size, thread->time);

    // sem_post(sharedMemorySemaphore);  // Liberar el semáforo después de modificar
    // sleep(thread->time);  // Simular el tiempo de ejecución

    //Calls dellocate
    deallocateProcess(thread);
    
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

void start() {
    int dist = generateRandomNumber(30,60);
    while(threadsQuantity < 50){
        createThread();
    }
}

int main() {

    //1-ask for the algorithm

    srand(time(NULL));
    //2- Memory 
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

    
    //WHILE
    start();
    

    // 3- Gererar random para la distribucion de procesos
    // 4- Creamos thread 
    // 5- Asignamos memoria (allocation algorithm)

    //---------------------------
    
    // int num_threads = 5;  // Suponiendo que queremos crear 5 hilos
    // for (int i = 0; i < num_threads; i++) {
    //     createThread();
    // }

    
    sem_close(sharedMemorySemaphore);
    shmdt(sharedControlMemoryPointer);

    return 0;
}
