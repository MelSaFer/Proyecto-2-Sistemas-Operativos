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
#include <semaphore.h>

sem_t *sharedMemorySemaphore, *logsSemaphore;




/*-----------------------------------------------------------------------
Funtion for destroy the shared memory
Entries: 
        -> key_t processSharedMemoryKey: key for the shared memory of process
        -> key_t controlSharedMemory: key for the shared memory of control
Output:
        -> 0 if the shared memory was destroyed successfully
        -> 1 if the shared memory was not destroyed successfully
*/
int finishEnvironment(key_t processSharedMemoryKey, key_t controlSharedMemory) {
    int shmid;
    shmid = shmget(processSharedMemoryKey, 0, 0644);
    if (shmid == -1) {
        perror("1-Error obtaining the shared memory ID");
        return 1;  
    }

    if (shmctl(shmid, IPC_RMID, NULL) == -1) {
        perror("2-Error destroying the shared memory");
        return 1;  
    }
    printf("\nShared memory destroyed!!!\n");
    shmid = shmget(controlSharedMemory, 0, 0644);
    if (shmid == -1) {
        perror("Error obtaining the shared memory ID");
        return 1;  
    }

    if (shmctl(shmid, IPC_RMID, NULL) == -1) {
        perror("Error destroying the shared memory");
        return 1;  
    }
    printf("\nControl memory destroyed!!!\n");

    return 0;
}



/*-----------------------------------------------------------------------
Funtion for get the keys of the shared memory
Entries: 
        -> None
Output:
        -> 0 if the keys were obtained successfully
        -> 1 if the keys were not obtained successfully
---------------------------------------------------------------------------*/
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

    return finishEnvironment(processSharedMemoryKey, controlSharedMemory);
}


int main() {
    int result = getKeys();
    return result;
}
