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
#include <time.h>
#include <poll.h>
#include "../include/sharedMem.h"
#include "../include/thread.h"
#include "../include/mainThread.h"

#define MIN_DISTRIBUTION 30
#define MIN_TIME 20
#define MAX_TIME 60
#define MIN_SIZE 1
#define MAX_SIZE 10
#define SLEEP_TIME 5
#define MAX_LINES 100
#define SHARED_MEMORY "files/shared_mem"
#define SHARED_MEMORY_ID 1

struct SHAREDMEM *sharedControlMemoryPointer;
FILE *file;
sem_t *sharedMemorySemaphore;
sem_t *logsSemaphore;

int threadsQuantity = 0;
int algorithm = 0;

// Function prototypes
int generateRandomNumber(int min, int max);
void paintMemory();
bool firstFit(void *arg);
bool bestFit(void *arg);
bool worstFit(void *arg);
void registerProcess(void *arg, int action);
void *allocateMainProcess(void *arg);
void *allocateProcess(void *arg);
void deallocateProcess(void *arg);
void createThread();
void createMainThread();
void accessSharedMemory();
int printAlgorithmMenu();
void start();

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
void paintMemory() {
    for (int i = 0; i < sharedControlMemoryPointer->lines; i++) {
        if (sharedControlMemoryPointer->partitions[i].pid == -1) {
            printf("- ");
        } else {
            printf("%d ", sharedControlMemoryPointer->partitions[i].pid);
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
bool firstFit(void *arg) {
    struct THREAD *thread = (struct THREAD *)arg;

    int size = thread->size;
    int consecutives = 0;
    int start = 0;

    for (int i = 0; i < sharedControlMemoryPointer->lines; i++) {
        if (sharedControlMemoryPointer->partitions[i].pid == -1) {
            consecutives++;
            if (consecutives == size) {
                sem_wait(sharedMemorySemaphore);
                for (int j = start; j < start + size; j++) {
                    sharedControlMemoryPointer->partitions[j] = *thread;
                }
                sem_post(sharedMemorySemaphore);
                sleep(SLEEP_TIME);
                printf("    Space assigned for process %d in line %d\n", thread->pid, start);
                return true;
            }
        } else {
            consecutives = 0;
            start = i + 1;
        }
    }
    printf("    There is not enough space for process %d\n", thread->pid);
    return false;
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
-------------------------------------------------------------------------*/
bool bestFit(void *arg) {
    struct THREAD *thread = (struct THREAD *)arg;
    
    int bestEmptyLine = -1;
    int bestEmptyCounter = MAX_LINES;
    int currentEmptyLine = 0;
    int startEmptyLine = 0;

    for (int i = 0; i < sharedControlMemoryPointer->lines; i++) {
        if (sharedControlMemoryPointer->partitions[i].pid == -1) {
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

    if (currentEmptyLine >= thread->size && currentEmptyLine < bestEmptyCounter) {
        bestEmptyLine = startEmptyLine;
        bestEmptyCounter = currentEmptyLine;
    }

    if (bestEmptyLine == -1) {
        printf("    There is not enough space for process %d\n", thread->pid);
        return false;
    } else {
        printf("    Space assigned for process %d in line %d\n", thread->pid, bestEmptyLine);
        sem_wait(sharedMemorySemaphore);
        for (int i = bestEmptyLine; i < bestEmptyLine + thread->size; i++) {
            sharedControlMemoryPointer->partitions[i] = *thread;
        }
        sem_post(sharedMemorySemaphore);
        sleep(SLEEP_TIME);
    }
    return true;
}

/*----------------------------------------------------
Worst Fit Algorithm
Entries:
   void *arg: thread data

Description:
   This function is in charge of allocating the thread in the memory using the worst fit algorithm.
   The worst fit algorithm assigns the thread to the partition with the most space available so that
   the thread can be allocated in the largest partition possible, leaving the smallest possible partitions.
----------------------------------------------------*/
bool worstFit(void *arg) {
    struct THREAD *thread = (struct THREAD *)arg;

    int startIndex = -1;
    int counter = 0;
    int maxNum = 0;
    int maxNumIndex = 0;
    bool first = true;

    for (int i = 0; i < sharedControlMemoryPointer->lines; i++) {
        if (sharedControlMemoryPointer->partitions[i].pid == -1) {
            if (first) {
                startIndex = i;
                first = false;
            }
            counter++;
        } else {
            if (counter > maxNum) {
                maxNum = counter;
                maxNumIndex = startIndex;
            }
            counter = 0;
            first = true;
        }
    }

    if (counter > maxNum) {
        maxNum = counter;
        maxNumIndex = startIndex;
    }

    if (maxNum < thread->size) {
        printf("    There is not enough space for process %d\n", thread->pid);
        return false;
    }

    sem_wait(sharedMemorySemaphore);
    int end = maxNumIndex + thread->size;
    for (int i = maxNumIndex; i < end; i++) {
        sharedControlMemoryPointer->partitions[i] = *thread;
    }
    sem_post(sharedMemorySemaphore);
    sleep(SLEEP_TIME);
    printf("    Space assigned for process %d in line %d\n", thread->pid, maxNumIndex);

    return true;
}

/*----------------------------------------------------
Function: registerProcess
Description:
   This function is in charge of registering the thread in the log file
   It writes the thread data to the log file
Entries:
   void *arg: thread data
Output:
   void
----------------------------------------------------*/
void registerProcess(void *arg, int action){
    file = fopen("log.txt", "a");
    if (!file) {
        fprintf(stderr, "Failed to open log file\n");
        return;
    }

    struct THREAD *thread = (struct THREAD *)arg;
    if(action == 0){
        printf("    Registering process %d with time %d\n", thread->pid, thread->time);
    }
    else{
        printf("    Ending process %d\n", thread->pid);
    }
    
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
    fclose(file);
}

/*----------------------------------------------------
Funtion for allocate the main process
Entries:
    void *arg: thread data
Output:
    void
----------------------------------------------------*/
void *allocateMainProcess(void *arg) {

    struct MAIN_THREAD *thread = (struct MAIN_THREAD *)arg;
    //printf("Distribution: %d\n", thread->distribution);

    while (1) {

        createThread();
        sleep(thread->distribution);
        
    }

    return NULL;
}

/*----------------------------------------------------
Function for allocate the process in memory
Entries:
    void *arg: thread data
Description:
    It uses the selected algorithm to allocate the thread in the memory.
----------------------------------------------------*/
void *allocateProcess(void *arg) {
    bool allocated = false;
    struct THREAD *thread = (struct THREAD *)arg;
    // thread->state = BLOCKED;
    printf("    Process %d searching for memory\n", thread->pid);

    registerProcess(thread, 0);

    if (algorithm == 0) {
        allocated = firstFit(thread);
    } else if (algorithm == 1) {
        allocated = bestFit(thread);
    } else {
        allocated = worstFit(thread);
    }
    for (int i = 0; i < sharedControlMemoryPointer->lines; i++) {
        if (sharedControlMemoryPointer->partitions[i].pid == thread->pid) {
            if (allocated)
            {
                sem_wait(sharedMemorySemaphore);
                sharedControlMemoryPointer->partitions[i].state = RUNNING;
                sem_post(sharedMemorySemaphore);
                sleep(thread->time);
                sem_wait(sharedMemorySemaphore);
                sharedControlMemoryPointer->partitions[i].state = FINISHED;
                sem_post(sharedMemorySemaphore);
                break;
            }
            else
            {
                //sem_wait("sharedMemorySemaphore", 0);
                sem_wait(sharedMemorySemaphore);
                sharedControlMemoryPointer->partitions[i].state = DEAD;
                sem_post(sharedMemorySemaphore);
                break;
            }
            
        }
    }
    sleep(SLEEP_TIME);

    registerProcess(thread, 1);  
    deallocateProcess(thread);
    
    return NULL;
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
    sem_wait(sharedMemorySemaphore);
    for (int i = 0; i < sharedControlMemoryPointer->lines; i++) {
        if (sharedControlMemoryPointer->partitions[i].pid == thread->pid) {
            sharedControlMemoryPointer->partitions[i].pid = -1;
        }
    }
    sem_post(sharedMemorySemaphore);
    // createThread();
    //free(thread);
}

/*----------------------------------------------------
Function for creating a new thread for the process
Entries:
    None
output:
    void
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
    data->state = BLOCKED;

    pthread_t thread;
    if (pthread_create(&thread, NULL, allocateProcess, (void *)data) != 0) {
        fprintf(stderr, "Failed to create thread\n");
        free(data);
    } else {
        pthread_detach(thread);
        ++threadsQuantity;
    }
}

/*----------------------------------------------------
Function: createMainThread
Description:
   This function is in charge of creating the main thread that generates the processes
----------------------------------------------------*/
void createMainThread() {
    struct MAIN_THREAD *data = malloc(sizeof(struct MAIN_THREAD));
    if (!data) {
        fprintf(stderr, "Failed to allocate memory for thread data\n");
        return;
    }

    data->distribution = generateRandomNumber(1, 5);

    pthread_t thread;
    if (pthread_create(&thread, NULL, allocateMainProcess, (void *)data) != 0) {
        fprintf(stderr, "Failed to create main thread\n");
    } else {
        pthread_detach(thread);
    }
}

/*----------------------------------------------------
Function: accessSharedMemory
Description:
   This function is in charge of accessing the shared memory and the semaphores
   It opens the shared memory and the semaphores to be used by the producer
----------------------------------------------------*/
void accessSharedMemory() {
    key_t key = ftok(SHARED_MEMORY, SHARED_MEMORY_ID);
    int shm_id = shmget(key, sizeof(struct SHAREDMEM), 0666);
    sharedControlMemoryPointer = (struct SHAREDMEM*) shmat(shm_id, NULL, 0);

    // Open log file
    file = fopen("log.txt", "w");
    if (!file) {
        fprintf(stderr, "Failed to open log file\n");
        return;
    }

    if (sharedControlMemoryPointer == (void*)-1) {
        //perror("shmat failed");
        fprintf(stderr, "Failed to attach shared memory, try to execute starter first\n");
        exit(EXIT_FAILURE);
    }

    sharedMemorySemaphore = sem_open("sharedMemorySemaphore", 0);
    if (sharedMemorySemaphore == SEM_FAILED) {
        //perror("sem_open failed");
        fprintf(stderr, "Failed to attach shared memory, try to execute starter first\n");
        exit(EXIT_FAILURE);
    }

    // Open log semaphore
    logsSemaphore = sem_open("logsSemaphore", 0);
    if (logsSemaphore == SEM_FAILED) {
        perror("sem_open failed");
        exit(EXIT_FAILURE);
    }
    fclose(file);
}

/*----------------------------------------------------
Function: printAlgorithmMenu
Description:
   This function prints the menu to select the memory allocation algorithm
   It reads the user input and returns the selected algorithm
Output:
    int: selected algorithm
----------------------------------------------------*/
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

    while (choice < 1 || choice > 3) {
        printf("Entrada inválida. Por favor, seleccione una opción válida (1-3): ");
        scanf("%d", &choice);
    }

    return choice - 1;
}

/*----------------------------------------------------
Funtion for free the memory
Entries:
    None
Output:
    void
----------------------------------------------------*/
void freeMemory() {
    sem_wait(sharedMemorySemaphore);
    for (int i = 0; i < sharedControlMemoryPointer->lines; i++) {
        sharedControlMemoryPointer->partitions[i].pid = -1;
    }
    sem_post(sharedMemorySemaphore);

    sem_close(sharedMemorySemaphore);
    sem_close(logsSemaphore);
    
}

/*----------------------------------------------------
Function: start
Description:
    This function is in charge of starting the main thread and waiting for the user to press a key to stop the program
    It creates the main thread and waits for the user to press a key to stop the program
----------------------------------------------------*/
void start() {
    createMainThread();

    printf("Presiona Enter para detener la creación de hilos:\n");

    struct pollfd stdin_fd;
    stdin_fd.fd = STDIN_FILENO;
    stdin_fd.events = POLLIN;

    while (1) {
        int ret = poll(&stdin_fd, 1, 0);
        if (ret == -1) {
            perror("poll");
            exit(EXIT_FAILURE);
        } else if (ret > 0) {
            if (stdin_fd.revents & POLLIN) {
                // key pressed - exit
                break;
            }
        }
    }

    freeMemory();
    printf("Se ha presionado Enter. Saliendo...\n");

}

// Main function
int main() {
    accessSharedMemory();
    algorithm = printAlgorithmMenu();
    system("clear");
    srand(time(NULL));
    
    start();

    sem_close(sharedMemorySemaphore);
    shmdt(sharedControlMemoryPointer);

    return 0;
}
