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

int finishEnvironment(key_t processSharedMemoryKey, key_t controlSharedMemory) {
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

    // Obtener el identificador de la memoria compartida
    shmid = shmget(controlSharedMemory, 0, 0644);
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
    key_t processSharedMemoryKey = 19070989;
    key_t controlSharedMemory = 19070987;
    finishEnvironment(processSharedMemoryKey, controlSharedMemory);
    return 0;
}