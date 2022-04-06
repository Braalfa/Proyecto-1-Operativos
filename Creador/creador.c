#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <semaphore.h>
#include <time.h>
#include "../data.h"
#include<sys/ipc.h>
#include<sys/shm.h>
#include<sys/types.h>
#include<string.h>
#include<errno.h>
#include<fcntl.h>

sem_t* clientSemaphore;
sem_t* reconstructorSemaphore;
sem_t* metadataSemaphore;
sem_t* finalizationSemaphore;
data *memoryAddress;
metaData *metadataStruct;
int memorySize;
int metadataID;
int sharedDataID;
int textSize = 1000;

void createMetadata();
void createSharedMemory();
void createMetadataSemaphore();
void createSharedSemaphores();
void wait(sem_t* semaphore);
void removeMemory();

int main(int argc, char** argv)
{  

    time_t start, end;
    time(&start);
    createMetadataSemaphore();
    createMetadata();
    createSharedMemory();
    createSharedSemaphores();

    sem_wait(finalizationSemaphore);

    // Esto va en finalizador
    sem_close(clientSemaphore);
    sem_close(reconstructorSemaphore);
    sem_close(metadataSemaphore);
    sem_unlink(CLIENT_SEMAPHORE);    
    sem_unlink(RECONSTRUCTOR_SEMAPHORE);    
    sem_unlink(METADATA_SEMAPHORE);
    sem_unlink(FINALIZATION_SEMAPHORE);
    removeMemory();
    return 0;
}

void removeMemory(){
    shmctl(metadataID, IPC_RMID, NULL);
    shmctl(sharedDataID, IPC_RMID, NULL);
}

void createMetadata(){
    metadataID = shmget(METADATA_KEY, sizeof(metaData), 0644|IPC_CREAT);
    if (metadataID < 0) {
        perror("shmget error\n");
        exit(1);
    }

    metadataStruct = shmat(metadataID, NULL, 0);

    wait(metadataSemaphore);
    // La siguiente linea define el tamano del array
    metadataStruct->sharedMemorySize = 15;
    memorySize = metadataStruct->sharedMemorySize;
    sem_post(metadataSemaphore);
}

void createSharedMemory(){
    sharedDataID = shmget(MEMORY_KEY, memorySize*sizeof(data), 0644|IPC_CREAT);
    if (sharedDataID < 0) {
        perror("shmget error\n");
        exit(1);
    }
    memoryAddress = shmat(sharedDataID, NULL, 0);
}

void createSharedSemaphores(){
    clientSemaphore = sem_open(CLIENT_SEMAPHORE, O_CREAT, 0644, memorySize);
    reconstructorSemaphore = sem_open(RECONSTRUCTOR_SEMAPHORE, O_CREAT, 0644, 0);
    finalizationSemaphore = sem_open(FINALIZATION_SEMAPHORE, O_CREAT, 0644, 0);
}

void createMetadataSemaphore(){
    metadataSemaphore = sem_open(METADATA_SEMAPHORE, O_CREAT, 0644, 1);
}