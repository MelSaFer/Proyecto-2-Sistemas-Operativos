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
#include "../include/sharedMem.h"
#include "../include/thread.h"

#define SHARED_MEMORY "files/shared_mem"
#define SHARED_MEMORY_ID 1


struct SHAREDMEM* sharedControlMemorySpy;
sem_t *sharedMemorySemaphore;


/*----------------------------------------------------
Function: paintMemory
Description:
   This function is in charge of printing the memory partitions and the threads assigned to each partition.
   For debugging purposes at the moment.
   If a partition is empty, it prints a -, otherwise it prints the thread ID.
----------------------------------------------------*/
void paintMemory(){
    printf("Spy: %d\n", sharedControlMemorySpy->lines);
    
    for (int i = 0; i < sharedControlMemorySpy->lines; i++) {
        if (sharedControlMemorySpy->partitions[i].pid == -1) {
            printf("- ");
        } else {
            //printf("%d ", sharedControlMemorySpy->partitions[i]);
            printf("%d ", sharedControlMemorySpy->partitions[i].pid);
        }
    }
    printf("\n");
}


/*----------------------------------------------------
Funtion for access to shared memory


*/
void accessSharedMemory(){
    key_t key = ftok(SHARED_MEMORY, SHARED_MEMORY_ID);
    int shm_id = shmget(key, sizeof(struct SHAREDMEM), 0666);
    sharedControlMemorySpy = (struct SHAREDMEM*) shmat(shm_id, NULL, 0);

    if (sharedControlMemorySpy == (void*)-1) {
        perror("shmat failed");
        exit(EXIT_FAILURE);
    }

    // Open the semaphore
    sharedMemorySemaphore = sem_open("sharedMemorySemaphore", 0);
    if (sharedMemorySemaphore == SEM_FAILED) {
        perror("sem_open failed");
        exit(EXIT_FAILURE);
    }
    // print sharedControlMemorySpy->lines
    printf("Spy: %d\n", sharedControlMemorySpy->lines);
}


//main function
int main(){
    accessSharedMemory();
    while (true) {
        sem_wait(sharedMemorySemaphore);
        paintMemory();
        sem_post(sharedMemorySemaphore);
        sleep(1);
    }
   return 0; 
}
