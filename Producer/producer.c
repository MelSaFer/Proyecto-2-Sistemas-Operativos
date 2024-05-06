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

//Prints the pids of the threads in memory
void printMemory(){
    for(int i = 0; i < sharedControlMemoryPointer->lines; i++){
        if(sharedControlMemoryPointer->partitions[i] != NULL){
            printf("Thread %d in partition %d\n", sharedControlMemoryPointer->partitions[i]->pid, i);
        }
    }
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

void worstFit(void *arg){

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
    //return NULL;
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
