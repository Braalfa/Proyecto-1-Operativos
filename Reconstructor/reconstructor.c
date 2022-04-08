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
#include <sys/resource.h>

long secondsBlocked = 0;
long timeUserMode;
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

void addFinalMetadata();
void loadMetadata();
void loadMetadataSemaphore();
void loadSharedSemaphores();
void loadSharedMemory();
void readTextFromMemory();
int hasClientFinished();
void wait(sem_t* semaphore);
void loadCharFromDataAddress(char* text, data* dataAddress, int position);
data* obtainNextDataAddress(data* currentDataAddress, int counter);

int main()
{
    loadMetadataSemaphore();
    loadMetadata();
    loadSharedMemory();
    loadSharedSemaphores();
    readTextFromMemory();

    struct rusage usage;
    getrusage(RUSAGE_SELF, &usage); 
    timeUserMode = usage.ru_utime.tv_sec*1000000 + usage.ru_utime.tv_usec;

    addFinalMetadata();
    sem_post(finalizationSemaphore);
    printf("\n\n");
    return 0;
}

void addFinalMetadata(){
    wait(metadataSemaphore);
    metadataStruct->reconstructorBlockedSeconds = secondsBlocked;
    metadataStruct->reconstructorUserModeMicroSeconds = timeUserMode;
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

void loadMetadata(){
    int shmid;

    shmid = shmget(METADATA_KEY, sizeof(metaData), 0644|IPC_CREAT);

    if (shmid < 0) {
        perror("shmget error\n");
        exit(1);
    }

    metadataStruct = shmat(shmid, NULL, 0);

    wait(metadataSemaphore);
    memorySize = metadataStruct->sharedMemorySize;
    sem_post(metadataSemaphore);
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

void readTextFromMemory(){
    int counter = 0;
    data* currentDataAddress = memoryAddress;
    int finished = 0;
    char text[textSize];

    while(!(reconstructorSemaphore->__align==0 && finished)){
        getchar();
        wait(reconstructorSemaphore);
        finished = hasClientFinished();
        loadCharFromDataAddress(text, currentDataAddress, counter);
        sem_post(clientSemaphore);
        counter += 1;
        currentDataAddress = obtainNextDataAddress(currentDataAddress, counter);
        printf("Reconstruccion: %s\n", text);
    }
}


int hasClientFinished(){
    wait(metadataSemaphore);
    int finished = metadataStruct->clienteFinished;
    sem_post(metadataSemaphore);
    return finished;
}

void wait(sem_t* semaphore){
    struct timespec before, after;
    clock_gettime(CLOCK_REALTIME, &before);
    sem_wait(semaphore);
    clock_gettime(CLOCK_REALTIME, &after);
    secondsBlocked += after.tv_sec - before.tv_sec;
}

void loadCharFromDataAddress(char* text, data* dataAddress, int position){
    *(text+position) = dataAddress->character;
    *(text+position+1) = '\0';
    char time[64];
    strftime(time, sizeof(time), "%c", &dataAddress->time);
    printf("Caracter Obtenido: %c \nFecha de insercion: %s\nPosicion: %li\n",
           dataAddress->character, time, dataAddress-memoryAddress);
}

data* obtainNextDataAddress(data* currentDataAddress, int counter){
    if (counter%memorySize==0){
        return memoryAddress;
    }else{
        return currentDataAddress + 1;
    }
}