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
    int shmid;
    shmid = shmget(processSharedMemoryKey, 0, 0644);
    if (shmid == -1) {
        perror("1-Error obtaining the shared memory ID");
        return 1;  // Cambiado de exit(1) a return 1 para un mejor manejo de errores
    }

    if (shmctl(shmid, IPC_RMID, NULL) == -1) {
        perror("2-Error destroying the shared memory");
        return 1;  // Cambiado de exit(1) a return 1
    }
    printf("\nShared memory destroyed!!!\n");
    shmid = shmget(controlSharedMemory, 0, 0644);
    if (shmid == -1) {
        perror("Error obtaining the shared memory ID");
        return 1;  // Cambiado de exit(1) a return 1
    }

    if (shmctl(shmid, IPC_RMID, NULL) == -1) {
        perror("Error destroying the shared memory");
        return 1;  // Cambiado de exit(1) a return 1
    }
    printf("\nShared memory destroyed!!!\n");

    

    return 0;
}

int getKeys() {
    const char *path1 = "files/process_mem";
    const char *path2 = "files/shared_mem";
    int id1 = 1, id2 = 1; 

    key_t processSharedMemoryKey = ftok(path1, id1);
    key_t controlSharedMemory = ftok(path2, id2);

    if (processSharedMemoryKey == -1 || controlSharedMemory == -1) {
        perror("ftok failed");
        return 1;
    }

    printf("\n 1- Shared memory key: %d\n", processSharedMemoryKey);
    printf("\n 2- control memory key: %d\n", controlSharedMemory);

    return finishEnvironment(processSharedMemoryKey, controlSharedMemory);
}

int main() {
    int result = getKeys();
    return result;
}
