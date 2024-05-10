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

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <pthread.h>
#include <semaphore.h>
#include <time.h>
#include <stdbool.h>
#include <sys/ipc.h>
#include <sys/shm.h>

#define MIN_TIME 20
#define MAX_TIME 60
#define MIN_SIZE 1
#define MAX_SIZE 10
#define MAX_LINES 100
#define SHARED_MEMORY "files/shared_mem"
#define SHARED_MEMORY_ID 1

// struct THREAD {
//     int pid;
//     int size;
//     int time;
// };

// struct SHAREDMEM {
//     int lines;
//     struct THREAD partitions[MAX_LINES];
// };

struct SHAREDMEM *sharedControlMemoryPointer;
sem_t *sharedMemorySemaphore;

int threadsQuantity = 0;
int algorithm = 0;

// Function prototypes
void createThread();
int generateRandomNumber(int min, int max);
void paintMemory();
void firstFit(void *arg);
void bestFit(void *arg);
void worstFit(void *arg);
void *allocateProcess(void *arg);
void deallocateProcess(void *arg);
void accessSharedMemory();
int printAlgorithmMenu();

int generateRandomNumber(int min, int max) {
    return min + rand() % (max - min + 1);
}

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

void firstFit(void *arg) {
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
                paintMemory();
                return;
            }
        } else {
            consecutives = 0;
            start = i + 1;
        }
    }
}

void bestFit(void *arg) {
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
        printf("No hay espacio suficiente para el proceso %d\n", thread->pid);
    } else {
        printf("Espacio asignado para el proceso %d en la línea %d\n", thread->pid, bestEmptyLine);
        sem_wait(sharedMemorySemaphore);
        for (int i = bestEmptyLine; i < bestEmptyLine + thread->size; i++) {
            sharedControlMemoryPointer->partitions[i] = *thread;
        }
        sem_post(sharedMemorySemaphore);
        paintMemory();
    }
}

void worstFit(void *arg) {
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
        return;
    }

    paintMemory();

    sem_wait(sharedMemorySemaphore);
    int end = maxNumIndex + thread->size;
    for (int i = maxNumIndex; i < end; i++) {
        sharedControlMemoryPointer->partitions[i] = *thread;
    }
    sem_post(sharedMemorySemaphore);
}

void *allocateProcess(void *arg) {
    struct THREAD *thread = (struct THREAD *)arg;

    if (algorithm == 0) {
        firstFit(thread);
    } else if (algorithm == 1) {
        bestFit(thread);
    } else {
        worstFit(thread);
    }
    sleep(thread->time);  
    deallocateProcess(thread);
    
    return NULL;
}

void deallocateProcess(void *arg) {
    struct THREAD *thread = (struct THREAD *)arg;
    sem_wait(sharedMemorySemaphore);
    for (int i = 0; i < sharedControlMemoryPointer->lines; i++) {
        if (sharedControlMemoryPointer->partitions[i].pid == thread->pid) {
            sharedControlMemoryPointer->partitions[i].pid = -1;
        }
    }
    sem_post(sharedMemorySemaphore);
    paintMemory();
    createThread();
    free(thread);
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
        pthread_detach(thread);
        ++threadsQuantity;
    }
}

void accessSharedMemory() {
    key_t key = ftok(SHARED_MEMORY, SHARED_MEMORY_ID);
    int shm_id = shmget(key, sizeof(struct SHAREDMEM), 0666);
    sharedControlMemoryPointer = (struct SHAREDMEM*) shmat(shm_id, NULL, 0);

    if (sharedControlMemoryPointer == (void*)-1) {
        perror("shmat failed");
        exit(EXIT_FAILURE);
    }

    sharedMemorySemaphore = sem_open("sharedMemorySemaphore", 0);
    if (sharedMemorySemaphore == SEM_FAILED) {
        perror("sem_open failed");
        exit(EXIT_FAILURE);
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

    while (choice < 1 || choice > 3) {
        printf("Entrada inválida. Por favor, seleccione una opción válida (1-3): ");
        scanf("%d", &choice);
    }

    return choice - 1;
}

void start() {
    while (threadsQuantity < 4) {
        createThread();
        sleep(1);
    }
}

int main() {
    algorithm = printAlgorithmMenu();
    srand(time(NULL));
    accessSharedMemory();
    start();
    
    while (1) {}

    sem_close(sharedMemorySemaphore);
    shmdt(sharedControlMemoryPointer);

    return 0;
}
