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

long secondsBlocked = 0;
long secondsUserMode;
long transferedCharacters;
int memorySize;
const char* fileName = "./input.txt";
sem_t* clientSemaphore;
sem_t* reconstructorSemaphore;
sem_t* metadataSemaphore;
sem_t* finalizationSemaphore;
data *memoryAddress;
metaData *metadataStruct;
int textSize = 1000;

void loadMetadata();
void loadMetadataSemaphore();
void loadSharedSemaphores();
void loadSharedMemory();
void removeMemory();
void loadCharFromDataAddress(char* text, data* dataAddress, int position);
data* obtainNextDataAddress(data* currentDataAddress, int counter);
void printData();



int main(){
    loadSharedMemory();
    loadSharedSemaphores();
    loadMetadataSemaphore();
    loadMetadata();
   
    sem_wait(metadataSemaphore);
    sem_wait(finalizationSemaphore);
    printData();

    sem_post(metadataSemaphore);
    sem_post(finalizationSemaphore);

    sem_close(clientSemaphore);
    sem_close(reconstructorSemaphore);
    sem_close(metadataSemaphore);
    sem_close(finalizationSemaphore);
    sem_unlink(CLIENT_SEMAPHORE);    
    sem_unlink(RECONSTRUCTOR_SEMAPHORE);    
    sem_unlink(METADATA_SEMAPHORE);
    sem_unlink(FINALIZATION_SEMAPHORE);
    return 0;
}

void loadSharedMemory(){    
    int shmid;    
    shmid = shmget(MEMORY_KEY, memorySize*sizeof(data), 0644|IPC_CREAT);
    if (shmid < 0) {
        perror("shmget error\n");
        exit(1);
    }

    memoryAddress = shmat(shmid, NULL, 0);
}

void loadMetadata(){
    int shmid;

    shmid = shmget(METADATA_KEY, sizeof(metaData), 0644|IPC_CREAT);
    if (shmid < 0) {
        perror("shmget error\n");
        exit(1);
    }

    metadataStruct = shmat(shmid, NULL, 0);

    sem_wait(metadataSemaphore);
    memorySize = metadataStruct->sharedMemorySize;
    sem_post(metadataSemaphore);
}

void loadSharedSemaphores(){
    clientSemaphore = sem_open(CLIENT_SEMAPHORE, 0);
    reconstructorSemaphore = sem_open(RECONSTRUCTOR_SEMAPHORE, 0);
    finalizationSemaphore = sem_open(FINALIZATION_SEMAPHORE, 0);
}

void loadMetadataSemaphore(){
    metadataSemaphore = sem_open(METADATA_SEMAPHORE, 0);
}


void printData(){
    printf("====================FINALIZADO====================\n");
    printf("Tiempo Bloqueado Cliente: %lu s\n", metadataStruct->clientBlockedSeconds);
    printf("Tiempo Bloqueado Reconstructor: %lu s\n", metadataStruct->reconstructorBlockedSeconds);
    printf("Caracter Transferidos: %d \n", metadataStruct->transferedCharacters);
    printf("Espacio Total de Memoria Utilizado (Datos): %d \n", metadataStruct->totalMemorySpace);
    printf("Tiempo Total en Modo Usuario Cliente: %lu microsegundos\n", metadataStruct->clientUserModeMicroSeconds);
    printf("Tiempo Total en Modo Usuario ReConstructor: %lu microsegundos\n", metadataStruct->reconstructorUserModeMicroSeconds);
    printf("==================================================\n");
    return;
}

