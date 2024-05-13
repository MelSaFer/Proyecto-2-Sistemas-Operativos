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

/*-----------------------------------------------------------------------
Funtion for destroy the shared memory
Entries: 
        -> key_t processSharedMemoryKey: key for the shared memory of process
        -> key_t controlSharedMemory: key for the shared memory of control
Return:
        -> 0 if the shared memory was destroyed successfully
        -> 1 if the shared memory was not destroyed successfully
*/
int finishEnvironment(key_t controlSharedMemory) {
    int shmid;
    
    shmid = shmget(controlSharedMemory, 0, 0644);
    if (shmid == -1) {
        perror("Error obtaining the shared memory ID");
        return 1;  
    }

    if (shmctl(shmid, IPC_RMID, NULL) == -1) {
        perror("Error destroying the shared memory");
        return 1;  
    }
    printf("\nShared memory destroyed!!!\n");

    

    return 0;
}

int getKeys() {
    const char *path2 = "files/shared_mem";
    int id1 = 1, id2 = 1; 

    key_t controlSharedMemory = ftok(path2, id2);

    if (controlSharedMemory == -1) {
        perror("ftok failed");
        return 1;
    }

    return finishEnvironment(controlSharedMemory);
}

int main() {
    int result = getKeys();
    if(result == 0) {
        printf("\nEnvironment destroyed successfully!!!\n");
    } else {
        printf("Error destroying environment\n");
    }
    return result;
}
