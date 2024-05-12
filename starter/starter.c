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

#include <sys/ipc.h>
#include <sys/shm.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <semaphore.h>
#include <fcntl.h>
#include <errno.h>
#include "../include/sharedMem.h"
#include "../include/thread.h"


#define PROCESS_SHARED_MEMORY "files/process_mem"
#define PROCESS_SHARED_MEMORY_ID 1

#define SHARED_MEMORY "files/shared_mem"
#define SHARED_MEMORY_ID 1

// Shared Memory Ids for the process
int processSharedMemId;
int controlSharedMemoryId;
// Shared Memory Keys for the process
key_t processSharedMemoryKey;
key_t controlSharedMemoryKey;
// Process Shared Memory
struct SHAREDMEM* sharedMemory;

sem_t *sharedMemorySemaphore, *logsSemaphore;

/*-----------------------------------------------
Function to start the environment
Entry:
    int lines -> number of lines to be created
Output:
    int -> 0 if the environment was created successfully, 1 otherwise
------------------------------------------------*/
int startEnvironment(int lines) {

    const char *filepath = SHARED_MEMORY;
    
    // Create the file
    int fd = open(filepath, O_RDWR | O_CREAT, 0666);
    if (fd == -1) {
        perror("Failed to open file");
        exit(EXIT_FAILURE);
    }
    close(fd);

    // Create the key we are going to use for the shared memory
    processSharedMemoryKey = ftok(PROCESS_SHARED_MEMORY, PROCESS_SHARED_MEMORY_ID);
    // Validate the creation of the key
    if (processSharedMemoryKey == -1) {
        perror("Error creating the key for the shared memory");
        exit(1);
    }

    // Create the shared memory for the process
    processSharedMemId = shmget(processSharedMemoryKey, sizeof(struct THREAD) * lines, 0666 | IPC_CREAT | IPC_EXCL);
    // Validate the creation of the shared memory
    if (processSharedMemId < 0) {
        // If the shared memory already exists, get it
        if (errno == EEXIST) {
            // Get the shared memory
            processSharedMemId = shmget(processSharedMemoryKey, sizeof(struct THREAD) * lines, 0666);
            // Validate the shared memory
            if (processSharedMemId < 0) {
                perror("Error getting the shared memory:/)");
                exit(1);
            }
        } else {
            perror("Error creating the shared memory");
            exit(1);
        }
    } else {
        printf("\nControl memory segment created with id %d\n", processSharedMemId);
    }

    //--------------------------------------------------------------------------------	
    // Create the key we are going to use for the shared memory
    controlSharedMemoryKey = ftok(SHARED_MEMORY, SHARED_MEMORY_ID);
    // Validate the creation of the key
    if (controlSharedMemoryKey == -1) {
        perror("Error creating the key for the control shared memory");
        exit(1);
    }

    // Create the shared memory for the memory
    controlSharedMemoryId = shmget(controlSharedMemoryKey, sizeof(struct SHAREDMEM), 0666 | IPC_CREAT | IPC_EXCL);
    // Validate the creation of the shared memory
    if (controlSharedMemoryId < 0) {
        // If the shared memory already exists, get it
        if (errno == EEXIST) {
            // Get the shared memory
            controlSharedMemoryId = shmget(controlSharedMemoryKey, sizeof(struct SHAREDMEM), 0666);
            // Validate the shared memory
            if (controlSharedMemoryId < 0) {
                perror("Error getting the shared memory:/)");
                exit(1);
            }
        } else {
            perror("Error creating the shared memory");
            exit(1);
        }
    } else {
        printf("\nShared memory segment created with id %d\n", controlSharedMemoryId);
    }

    // Attach shared control memory
    sharedMemory = (struct SHAREDMEM*) shmat(controlSharedMemoryId, NULL, 0);
    // Validate shared control memory
    if (sharedMemory == (void *) -1) {
        perror("shmat");
        exit(1);
    }

    // Set initial values to control structure of shared control memory
    sharedMemory->lines = lines;
    for (int i = 0; i < lines; i++) {
        sharedMemory->partitions[i].pid = -1;
    }

    // Close shared control memory
    shmdt(sharedMemory);

    // CREATE SEMAPHORES
    // Unlink first to ensure the semaphore can be created
    sem_unlink("sharedMemorySemaphore");
    sem_unlink("logsSemaphore");
    
    // Create binary semaphores
    sharedMemorySemaphore = sem_open("sharedMemorySemaphore", O_CREAT, 0644, 1);
    if (sharedMemorySemaphore == SEM_FAILED) {
        perror("Unable to create sharedMemorySemaphore");
        exit(1);
    }

    logsSemaphore = sem_open("logsSemaphore", O_CREAT, 0644, 1);
    if (logsSemaphore == SEM_FAILED) {
        perror("Unable to create logsSemaphore");
        exit(1);
    }

    printf("\nSemaphores created successfully!!\n");

    // Closing the semaphores
    sem_close(sharedMemorySemaphore);
    sem_close(logsSemaphore);
    
    return 0; // Return success

}

// Main function
int main() {
    // Input the number of lines
    int lines;

    printf("Enter the number of lines: ");
    scanf("%d", &lines);

    while(0 >= lines || lines > MAX_LINES){
        system("clear");
        printf("Invalid number of lines, please enter a number between 1 and %d\n", MAX_LINES);
        printf("Enter the number of lines: ");
        scanf("%d", &lines);
    }

    // Start the environment
    int state = startEnvironment(lines);

    if(state == 0) {
        printf("\nEnvironment started successfully!!\n");
    } else {
        printf("Error starting the environment\n");
    }

    return 0;
}