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
FILE *file;
sem_t *sharedMemorySemaphore;
sem_t *logsSemaphore;

int threadsQuantity = 0;
int algorithm = 0;

// Function prototypes
void createThread();

/*----------------------------------------------------
Function: generateRandomNumber
Entries:
   int min: minimum value of the random number
   int max: maximum value of the random number
Description:
   This function generates a random number between the minimum and maximum values provided.
   It uses the rand() function from the standard library to generate the random number.
----------------------------------------------------*/
int generateRandomNumber(int min, int max) {
    return min + rand() % (max - min + 1);
}

/*----------------------------------------------------
Function: paintMemory
Description:
   This function is in charge of printing the memory partitions and the threads assigned to each partition.
   For debugging purposes at the moment.
   If a partition is empty, it prints a -, otherwise it prints the thread ID.
----------------------------------------------------*/
void paintMemory(){
    for (int i = 0; i < sharedControlMemoryPointer->lines; i++) {
        if (sharedControlMemoryPointer->partitions[i] == NULL) {
            printf("- ");
        } else {
            printf("%d ", sharedControlMemoryPointer->partitions[i]->pid);
        }
    }
    printf("\n");
}

//ALGORITMS OF MEMORY ASSIGNATION 

/*----------------------------------------------------
First Fit Algorithm
Entries:
    void *arg: thread data
Description:
    This function is in charge of allocating the thread in the memory using the first fit algorithm.
    The first fit algorithm assigns the thread to the first partition that has enough space to allocate it.
----------------------------------------------------*/
bool firstFit(void *arg){
    struct THREAD *thread = (struct THREAD *)arg;

    int size = thread->size;
    int consecutives = 0;
    int start = 0;

    // Iterate through the array of partitions to find the first empty space
    for (int i = 0; i < sharedControlMemoryPointer->lines; i++){
        
        // if the partition is empty, increment the consecutive empty spaces
        if(sharedControlMemoryPointer->partitions[i] == NULL){
            consecutives++;

            // if the consecutive empty spaces are equal to the size of the process, assign the process to the memory
            if(consecutives == size){
                sem_wait(sharedMemorySemaphore);
                for(int j = start; j < start+size; j++){
                    sharedControlMemoryPointer->partitions[j] = thread;
                }
                sem_post(sharedMemorySemaphore);
                paintMemory();
                return true;
            }
        }
        // if the partition is not empty, reset the consecutive empty spaces and start from the next partition
        else{
            consecutives = 0;
            start = i + 1;
        }
    }

    return true;
}

/*-----------------------------------------------------------------------
Best Fit Algorithm
Description:
    This function is in charge of allocating the thread in the memory using 
    the best fit algorithm.The best fit algorithm assigns the thread to the 
    partition with the smallest space available that fits the thread.
Entries:
    void *arg: thread data
Output:
    void
-------------------------------------------------------------------------
*/
bool bestFit(struct THREAD *thread) {
    printf("Entering best Fit\n");
    int bestEmptyLine = -1;  // Index of the best fit line start
    int bestEmptyCounter = 100;  // Counter for the smallest empty line set found
    int currentEmptyLine = 0;  // Current counter of consecutive empty lines
    int startEmptyLine = 0;    // Start index of the current empty line set

    // Find the best fit iterating over the memory lines
    for (int i = 0; i < sharedControlMemoryPointer->lines; i++) {
        if (sharedControlMemoryPointer->partitions[i] == NULL) {
            if (currentEmptyLine == 0) {
                startEmptyLine = i;
            }
            currentEmptyLine++;
        } else {
            if (currentEmptyLine >= thread->size && currentEmptyLine < bestEmptyCounter) {
                bestEmptyLine = startEmptyLine;
                bestEmptyCounter = currentEmptyLine;
            }
            currentEmptyLine = 0;
        }
    }

    // Check the last sequence of empty lines because it could be the best fit
    if (currentEmptyLine >= thread->size && currentEmptyLine < bestEmptyCounter) {
        bestEmptyLine = startEmptyLine;
        bestEmptyCounter = currentEmptyLine;
    }

    printf("Memory before:");
    paintMemory();

    // Assign the process it there is enough space
    if (bestEmptyLine == -1) {
        printf("No hay espacio suficiente para el proceso %ld\n", (long)thread->pid);
        return false;
    } else {
        printf("Espacio asignado para el proceso %ld en la línea %d\n", (long)thread->pid, bestEmptyLine);
        //process info
        printf("Thread %d with size %d and time %d started.\n", thread->pid, thread->size, thread->time);
        
        sem_wait(sharedMemorySemaphore);  // Ensure exclusive access to memory
        for (int i = bestEmptyLine; i < bestEmptyLine + thread->size; i++) {
            sharedControlMemoryPointer->partitions[i] = thread;
        }
        sem_post(sharedMemorySemaphore);

        paintMemory();
        
    }
    return true;
}


/*----------------------------------------------------
Worst Fit Algorithm
Entries:
   void *arg: thread data
Output:
   bool - true if the thread was allocated, false otherwise

Description:
   This function is in charge of allocating the thread in the memory using the worst fit algorithm.
   The worst fit algorithm assigns the thread to the partition with the most space available so that
   the thread can be allocated in the largest partition possible, leaving the smallest possible partitions.
----------------------------------------------------*/
bool worstFit(void *arg){
    struct THREAD *thread = (struct THREAD *)arg;

    int startIndex = -1;
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
        return false;
    }
 
    sem_wait(sharedMemorySemaphore); // semaphore wait
    int end = maxNumIndex + thread->size;
    for (int i = maxNumIndex; i < end; i++) { // Allocate the thread in the each partition
        printf("Thread %d assigned to partition %d\n", thread->pid, i);
        sharedControlMemoryPointer->partitions[i] = thread;
    }
    sem_post(sharedMemorySemaphore);

    return true;
}




//----------------------------------------------------
// Function: registerProcess
// Description:
//    This function is in charge of registering the thread in the log file
//    It writes the thread data to the log file
// Entries:
//    void *arg: thread data
// Output:
//    void
//----------------------------------------------------
void registerProcess(void *arg, int action){
    struct THREAD *thread = (struct THREAD *)arg;
    printf("Registering process %d\n", thread->pid);

    sem_wait(logsSemaphore);  // wait
    if(action == 0){
        fprintf(file, "Process %d with size %d and time %d started running.\n", thread->pid, thread->size, thread->time);
    }else{
        if (thread->state == FINISHED){
            fprintf(file, "Process %d with size %d and time %d finished succesfully.\n", thread->pid, thread->size, thread->time);
        }else if (thread->state == DEAD){
            fprintf(file, "Process %d with size %d and time %d couldn't enter memory.\n", thread->pid, thread->size, thread->time);
        }
    }
    sem_post(logsSemaphore);  // free
}


/*----------------------------------------------------
Function: deallocateProcess
Description:
   This function is in charge of deallocating the thread from the memory.
   It iterates through the array of partitions to find the thread and deallocate it.
   It sets the partition to NULL to indicate that it is empty
Entries:
   void *arg: thread data
Output:
   void
----------------------------------------------------*/
void deallocateProcess(void *arg) {
    struct THREAD *thread = (struct THREAD *)arg;
    sem_wait(sharedMemorySemaphore);  // wait for the semaphore

    // Iterate through the array of partition to find the thread and deallocate it
    for (int i = 0; i < sharedControlMemoryPointer->lines; i++) {
        //printf("Partition %d\n", i);
        if (sharedControlMemoryPointer->partitions[i] != NULL && sharedControlMemoryPointer->partitions[i]->pid == thread->pid) {
            sharedControlMemoryPointer->partitions[i] = NULL;
            //printf("Thread %d deallocated from partition %d\n", thread->pid, i);
        }
    }
    sem_post(sharedMemorySemaphore);  // free the semaphore
    paintMemory();
    createThread();
    free(thread);  // Free the memory allocated for the thread
}   

/*----------------------------------------------------
Allocate the process in memory
Entries:
    void *arg: thread data
Description:
    It uses the selected algorithm to allocate the thread in the memory.
----------------------------------------------------*/
void *allocateProcess(void *arg) {
    bool allocated = false;
    
    struct THREAD *thread = (struct THREAD *)arg;
    thread->state = BLOCKED;
    registerProcess(thread, 0);

    if(algorithm == 0){
        allocated = firstFit(thread);
    }else if(algorithm == 1){
        allocated = bestFit(thread);
    }else{
        allocated = worstFit(thread);
        paintMemory();
    }

    if(allocated){
        thread->state = RUNNING;
        sleep(thread->time);
        thread->state = FINISHED;
    }else{
        thread->state = DEAD;
    }

    //Calls dellocate
    registerProcess(thread, 1);
    deallocateProcess(thread);
    
    
    return NULL;
}

/*----------------------------------------------------
Function: createThread
Description:
   This function is in charge of creating a new thread for the process
----------------------------------------------------*/
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
        //pthread_join(thread, NULL);
        pthread_detach(thread);
        ++threadsQuantity;
    }
}

// //
// void createThreads(int num_threads) {
//     for (int i = 0; i < num_threads; i++) {
//         createThread();
//     }
// }

/*----------------------------------------------------------------------------
Function: start
Description:
   This function is in charge of starting the process of creating threads
Entries:
   void
Output:
    void
--------------------------------------------------------------------------------
*/
void start() {
    // int dist = generateRandomNumber(30,60);
    while(threadsQuantity < 4){
        createThread();
        sleep(1);
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

/*----------------------------------------------------
Funtion for access to shared memory


*/
void accessSharedMemory(){
    key_t key = ftok(SHARED_MEMORY, SHARED_MEMORY_ID);
    int shm_id = shmget(key, sizeof(struct SHAREDMEM), 0666);
    sharedControlMemoryPointer = (struct SHAREDMEM*) shmat(shm_id, NULL, 0);

    file = fopen("log.txt", "w");
    if (!file) {
        fprintf(stderr, "Failed to open log file\n");
        return;
    }

    if (sharedControlMemoryPointer == (void*)-1) {
        perror("shmat failed");
        exit(EXIT_FAILURE);
    }

    // Open the semaphore
    sharedMemorySemaphore = sem_open("sharedMemorySemaphore", 0);
    if (sharedMemorySemaphore == SEM_FAILED) {
        perror("sem_open failed");
        exit(EXIT_FAILURE);
    }

    // Open log semaphore
    logsSemaphore = sem_open("logsSemaphore", 0);
    if (logsSemaphore == SEM_FAILED) {
        perror("sem_open failed");
        exit(EXIT_FAILURE);
    }
}

int main() {

    algorithm = printAlgorithmMenu();

    srand(time(NULL));
    
    accessSharedMemory();

    start();
    
    while (1)
    {
        /* code */
    }

    sem_close(sharedMemorySemaphore);
    shmdt(sharedControlMemoryPointer);

    return 0;
}

