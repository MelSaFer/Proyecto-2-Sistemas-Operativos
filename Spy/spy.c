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
This function is in charge of returning the name of the state of a thread.
Entries:
    enum State state -> the state of the thread
Output:
    const char* -> the name of the state of the thread
*/
const char* getStateName(enum State state) {
    switch (state) {
        case BLOCKED: return "BLOCKED";
        case RUNNING: return "RUNNING";
        case FINISHED: return "FINISHED";
        case DEAD: return "DEAD";
        default: return "UNKNOWN";  // En caso de que se agregue otro estado y no se actualice esta función
    }
}
/*----------------------------------------------------
Function: paintMemory
Description:
   This function is in charge of printing the memory partitions and the threads assigned to each partition.
   For debugging purposes at the moment.
   If a partition is empty, it prints a -, otherwise it prints the thread ID.
Entries:
    None
Output:
    void
----------------------------------------------------*/
void paintMemory(){
    system("clear");
    printf("Estado de la memoria:\n");
    printf("+--------------------------------+\n");
    
    for (int i = 0; i < sharedControlMemorySpy->lines; i++) {
        if (sharedControlMemorySpy->partitions[i].pid == -1) {
            printf("| %-30s |\n", "-");  
        } else {
            printf("| %-30d |\n", sharedControlMemorySpy->partitions[i].pid);  
        }
        printf("+--------------------------------+\n");
    } 
    printf("\n");
    
}


/*----------------------------------------------------
Prints the statistics of the threads
Entries:
    None
Output:
    void
-----------------------------------------------------*/
void printInfoProcess() {
    system("clear");
    int lastThreadId;	

    printf("Información de los Procesos:\n");
    printf("+-----+------+-------+-------------+\n");
    printf("| PID | Size | Time  | State       |\n");
    printf("+-----+------+-------+-------------+\n");

    for (int i = 0; i < sharedControlMemorySpy->lines; i++) {
        
        if(lastThreadId == sharedControlMemorySpy->partitions[i].pid){
            continue;
        } else if(sharedControlMemorySpy->partitions[i].pid == -1){
            continue;
        }
        lastThreadId = sharedControlMemorySpy->partitions[i].pid;
        printf("| %-3d | %-4d | %-5d | %-11s |\n",
                sharedControlMemorySpy->partitions[i].pid,
                sharedControlMemorySpy->partitions[i].size,
                sharedControlMemorySpy->partitions[i].time,
                //sharedControlMemorySpy->partitions[i].state);
                getStateName(sharedControlMemorySpy->partitions[i].state));
        printf("+-----+------+-------+-------------+\n");
    }
}

/*----------------------------------------------------
Funtion for access to shared memory
Entries:
    None
Output:
    void
----------------------------------------------------*/
void accessSharedMemory(){
    key_t key = ftok(SHARED_MEMORY, SHARED_MEMORY_ID);
    int shm_id = shmget(key, sizeof(struct SHAREDMEM), 0666);
    sharedControlMemorySpy = (struct SHAREDMEM*) shmat(shm_id, NULL, 0);

    if (sharedControlMemorySpy == (void*)-1) {
        //perror("shmat failed");
        fprintf(stderr, "Failed to attach shared memory, try to execute starter first\n");
        exit(EXIT_FAILURE);
    }

    // Open the semaphore
    sharedMemorySemaphore = sem_open("sharedMemorySemaphore", 0);
    if (sharedMemorySemaphore == SEM_FAILED) {
        //perror("sem_open failed");
        fprintf(stderr, "Failed to attach shared memory, try to execute starter first\n");
        exit(EXIT_FAILURE);
    }
    // print sharedControlMemorySpy->lines
    printf("Spy: %d\n", sharedControlMemorySpy->lines);
}

/*----------------------------------------------------
Funtion for print the menu of the spy
Entries:
    None
Output:
    int choice -> the option selected by the user
----------------------------------------------------*/
int printSpyMenu() {
    int choice;

    printf("\n+----------------------------------------------------+\n");
    printf("|         Sistema de Información de Procesos         |\n");
    printf("+----------------------------------------------------+\n");
    printf("| 1. Ver estado actual de la memoria                 |\n");
    printf("| 2. Ver estado de los procesos                      |\n");
    printf("| 3. Salir                                           |\n");
    printf("+----------------------------------------------------+\n");
    printf("Seleccione una opción (1-3): ");

    scanf("%d", &choice);

    while (choice < 1 || choice > 3) {
        printf("Entrada inválida. Por favor, seleccione una opción válida (1-3): ");
        scanf("%d", &choice);
    }

    return choice;
}


/*----------------------------------------------------
Main function
Entries:
    None
Output:
    int -> 0
----------------------------------------------------*/
int main(){
    accessSharedMemory();
    while (true) {
        int choice = printSpyMenu();

        if (choice == 1) {
            sem_wait(sharedMemorySemaphore);
            paintMemory();
            sem_post(sharedMemorySemaphore);
        } else if (choice == 2) {
            sem_wait(sharedMemorySemaphore);
            printInfoProcess();
            sem_post(sharedMemorySemaphore);
        }
        else if (choice == 3) {
            break;
        }
    }
   return 0; 
}
