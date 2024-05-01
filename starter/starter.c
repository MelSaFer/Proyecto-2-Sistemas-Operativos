#include <sys/ipc.h>
#include <sys/shm.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <semaphore.h>
#include <fcntl.h>
#include <errno.h>
#include "../sharedMem.h"
#include "../thread.h"


#define PROCESS_SHARED_MEMORY "shared_mem"
#define PROCESS_SHARED_MEMORY_ID 1

// Shared Memory Ids for the process
int processSharedMemId;
// Shared Memory Keys for the process
key_t processSharedMemoryKey;
// Process Shared Memory
struct SHAREDMEM* sharedMemory;

sem_t *sharedMemorySemaphore, *logsSemaphore;

//start Environment function
int startEnvironment(int lines) {

    const char *filepath = "shared_mem";
    
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
    printf("\nShared memory key: %d\n", processSharedMemoryKey);

    // Create the shared memory
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
            printf("\nExisting shared memory segment with id %d opened\n", processSharedMemId);
        } else {
            perror("Error creating the shared memory");
            exit(1);
        }
    } else {
        printf("\nShared memory segment created with id %d\n", processSharedMemId);
    }

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

    // Closing the semaphores
    sem_close(sharedMemorySemaphore);
    sem_close(logsSemaphore);

    printf("\nShared memory segment created with id %d\n", processSharedMemId);
    
    return 0; // Return success

}

int main() {
    startEnvironment(5);
    return 0;
}