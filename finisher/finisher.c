#include <sys/ipc.h>
#include <sys/shm.h>
#include <stdio.h>
#include <stdlib.h>

int finishEnvironment(key_t processSharedMemoryKey) {
    // Obtener el identificador de la memoria compartida
    int shmid = shmget(processSharedMemoryKey, 0, 0644);
    if (shmid == -1) {
        perror("Error obtaining the shared memory ID");
        exit(1);
    }

    // Destruir la memoria compartida
    if (shmctl(shmid, IPC_RMID, NULL) == -1) {
        perror("Error destroying the shared memory");
        exit(1);
    }
    printf("\nShared memory destroyed!!!\n");

    return 0;
}


int main() {
    key_t processSharedMemoryKey = 22337049;
    finishEnvironment(processSharedMemoryKey);
    return 0;
}