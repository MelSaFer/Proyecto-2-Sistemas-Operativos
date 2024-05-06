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
#include <stdbool.h>
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

// ----------------------------------------------------
// Function: paintMemory
// Description:
//    This function is in charge of printing the memory partitions and the threads assigned to each partition.
//    For debugging purposes at the moment.
//    If a partition is empty, it prints a 0, otherwise it prints the thread ID.
// ----------------------------------------------------
void paintMemory(){
    for (int i = 0; i < sharedControlMemoryPointer->lines; i++) {
        if (sharedControlMemoryPointer->partitions[i] == NULL) {
            printf("0 ");
        } else {
            printf("%d ", sharedControlMemoryPointer->partitions[i]->pid);
        }
    }
    printf("\n");
}

//ALGORITMS OF MEM ASSIGNATION
void firstFit(void *arg){

}

void bestFit(void *arg){
    // //sharedControlMemoryPointer
    // struct THREAD *thread = (struct THREAD *)arg;
    // //asignar el proceso en el primer espacio vacio que sea mayor o igual al tamaño del proceso
    // int i = 0;
    // while(i < sharedControlMemoryPointer->lines){
    //     printf("Buscando espacio en la particion %d\n", i);
    //     if(sharedControlMemoryPointer->partitions[i] == NULL){
    //         i++;
    //     }else{
    //         if(sharedControlMemoryPointer->partitions[i]->size >= thread->size){
    //             sem_wait(sharedMemorySemaphore);
    //             sharedControlMemoryPointer->partitions[i] = thread->pid;
    //             //prints the threads info
    //             sem_post(sharedMemorySemaphore);
    //             printf("Thread %d with size %d and time %d started.\n", thread->pid, thread->size, thread->time);
    //             break;
    //         }
    //         i++;
    //     }
    // }

}


//----------------------------------------------------
// Worst Fit Algorithm
// Entries:
//    void *arg: thread data
//
// Description:
//    This function is in charge of allocating the thread in the memory using the worst fit algorithm.
//    The worst fit algorithm assigns the thread to the partition with the most space available so that
//    the thread can be allocated in the largest partition possible, leaving the smallest possible partitions.
//----------------------------------------------------
void worstFit(void *arg){
    struct THREAD *thread = (struct THREAD *)arg;

    int startIndex = -1;
    int counter = 0;
    int maxNum = 0;
    int maxNumIndex = 0;
    bool first = true;
    bool empty = true;

    // Iterate through the array of partitions
    for (int i = 0; i < sharedControlMemoryPointer->lines; i++) {
        printf("Partition %d\n", i);
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
            printf("Counter: %d\n", counter);
            if (counter > maxNum) {
                maxNum = counter;
                maxNumIndex = startIndex;
            }
            counter = 0;
            first = true;
        }

    }

    // Last check to see if the last partition is the largest
    if (counter > maxNum) {
        maxNum = counter;
        maxNumIndex = startIndex;
    }

    // If the maxNum is less than the size of the thread, then the thread can't be allocated
    // and the function returns
    if (maxNum < thread->size) {
        return;
    }

    printf("MaxNum: %d\n", maxNum);
    printf("MaxNumIndex: %d\n", maxNumIndex);

    paintMemory();

    // semaphore wait
    sem_wait(sharedMemorySemaphore);
    // Allocate the thread in the each partition
    int end = maxNumIndex + thread->size;
    for (int i = maxNumIndex; i < end; i++) {
        // Assign the thread to the partition
        printf("Thread %d assigned to partition %d\n", thread->pid, i);
        sharedControlMemoryPointer->partitions[i] = thread;
    }
    // semaphore post
    sem_post(sharedMemorySemaphore);
}




//----------------------------------------------------

void registerProcess(void *arg){

}


// ----------------------------------------------------
// Function: deallocateProcess
// Entries:
//    void *arg: thread data
// Description:
//    This function is in charge of deallocating the thread from the memory.
//    It iterates through the array of partitions to find the thread and deallocate it.
//    It sets the partition to NULL to indicate that it is empty.
// ----------------------------------------------------
void deallocateProcess(void *arg) {
    struct THREAD *thread = (struct THREAD *)arg;
    sem_wait(sharedMemorySemaphore);  // Esperar por el semáforo antes de modificar la memoria compartida

    // Iterate through the array of partition to find the thread and deallocate it
    for (int i = 0; i < sharedControlMemoryPointer->lines; i++) {
        printf("Partition %d\n", i);
        if (sharedControlMemoryPointer->partitions[i] != NULL && sharedControlMemoryPointer->partitions[i]->pid == thread->pid) {
            sharedControlMemoryPointer->partitions[i] = NULL;
            printf("Thread %d deallocated from partition %d\n", thread->pid, i);
        }
    }
    sem_post(sharedMemorySemaphore);  // Liberar el semáforo después de modificar
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
    //Pirnt the thread info
    printf("Thread %d with size %d and time %d started.\n", thread->pid, thread->size, thread->time);
    if(algorithm == 0){
        firstFit(thread);
    }else if(algorithm == 1){
        printf("Best Fit\n");
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
        //sleep(dist);
    }
}

int printAlgorithmMenu() {
    int choice;

    printf("\n+---------------------------------+\n");
    printf("|   Seleccione un algoritmo de    |\n");
    printf("|     asignación de memoria:      |\n");
    printf("+---------------------------------+\n");
    printf("| 1. First-Fit                    |\n");
    printf("| 2. Best-Fit                     |\n");
    printf("| 3. Worst-Fit                    |\n");
    printf("+---------------------------------+\n");
    printf("Ingrese su elección (1-3): ");
    
    scanf("%d", &choice);

    // Validar la entrada del usuario
    while (choice < 1 || choice > 3) {
        printf("Entrada inválida. Por favor, seleccione una opción válida (1-3): ");
        scanf("%d", &choice);
    }

    return choice-1;
}

int main() {

    //1-ask for the algorithm
    algorithm = printAlgorithmMenu();

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
    //printf("Cantidad de lineas en memoria: %d\n", sharedControlMemoryPointer->lines);

    // Open the semaphore
    sharedMemorySemaphore = sem_open("sharedMemorySemaphore", 0);
    if (sharedMemorySemaphore == SEM_FAILED) {
        perror("sem_open failed");
        exit(EXIT_FAILURE);
    }

    
    //WHILE
    // start();

    // Test #1 worst fit: Allocate process when there's a process in the middle
    struct THREAD *thread = malloc(sizeof(struct THREAD));
    thread->size = 3;
    thread->time = 5;
    thread->pid = 1;
    worstFit(thread);
    paintMemory();

    struct THREAD *thread2 = malloc(sizeof(struct THREAD));
    thread2->size = 4;
    thread2->time = 5;
    thread2->pid = 2;
    worstFit(thread2);
    paintMemory();

    deallocateProcess(thread);
    paintMemory();

    struct THREAD *thread3 = malloc(sizeof(struct THREAD));
    thread3->size = 3;
    thread3->time = 5;
    thread3->pid = 3;
    worstFit(thread3);
    paintMemory();
    

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
