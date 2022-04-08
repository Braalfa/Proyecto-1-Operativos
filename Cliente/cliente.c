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
#include <sys/resource.h>

int textSize = 1000;
int memorySize;
const char* fileName = "./input.txt";

long secondsBlocked = 0;
long timeUserMode;
long transferedCharacters;
sem_t* clientSemaphore;
sem_t* reconstructorSemaphore;
sem_t* metadataSemaphore;
sem_t* finalizationSemaphore;
data *memoryAddress;
metaData *metadataStruct;

void loadMetadata();
void loadSharedMemory();
void loadMetadataSemaphore();
void loadSharedSemaphores();
void addFinalMetadata();
int readFile(char* text);
void addTextToMemory(char* text);
void writeCharToDataAddress(char c, data* dataAddress);
data* obtainNextDataAddress(data* currentDataAddress, int counter);
void updateMetadataFinishedValue(int value);
void wait(sem_t* semaphore);

int main(int argc, char** argv)
{  
    if(argc>1){
        fileName = argv[1];
    }

    loadMetadataSemaphore();
    loadMetadata();
    loadSharedMemory();
    loadSharedSemaphores();

    char text[textSize];
    if(readFile(text) == 0){
        return 0;
    }
    addTextToMemory(text);

    struct rusage usage;
    getrusage(RUSAGE_SELF, &usage); 
    timeUserMode = usage.ru_utime.tv_sec*1000000 + usage.ru_utime.tv_usec;
    
    transferedCharacters = strlen(text);
    addFinalMetadata();
    
    printf("\n\n");
    return 0;
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

void loadSharedSemaphores(){
    clientSemaphore = sem_open(CLIENT_SEMAPHORE, 0);
    reconstructorSemaphore = sem_open(RECONSTRUCTOR_SEMAPHORE, 0);
    finalizationSemaphore = sem_open(FINALIZATION_SEMAPHORE, 0);
}

void loadMetadataSemaphore(){
    metadataSemaphore = sem_open(METADATA_SEMAPHORE, 0);
}

void addFinalMetadata(){
    wait(metadataSemaphore);
    metadataStruct->clientUserModeMicroSeconds = timeUserMode;
    metadataStruct->clientBlockedSeconds = secondsBlocked;
    metadataStruct->totalMemorySpace = metadataStruct->sharedMemorySize*sizeof(data)+sizeof(metaData);
    metadataStruct->transferedCharacters = transferedCharacters;
    sem_post(metadataSemaphore);
}

int readFile(char* text)
{
    int counter = 0;
    FILE *file = fopen(fileName, "r");
    int character;
    if (file == NULL){
        printf("No se encontró el archivo de lectura\n");
        return 0;
    }
    while ((character = fgetc(file)) != EOF)
    {
        text[counter] = (char) character;
        counter += 1;
    }
    text[counter] = '\0';
    return 1;
}

void addTextToMemory(char* text){
    int counter = 0;
    data* currentDataAddress = memoryAddress;
    while (text[counter] != '\0'){
        getchar();
        wait(clientSemaphore);
        writeCharToDataAddress(text[counter], currentDataAddress);
        counter += 1;
        updateMetadataFinishedValue(text[counter] == '\0');
        sem_post(reconstructorSemaphore);
        currentDataAddress = obtainNextDataAddress(currentDataAddress, counter);
        printf("Pendiente: %s\n", text+counter);
    }
}

void writeCharToDataAddress(char c, data* dataAddress){
    dataAddress->character = c;
    time_t t = time(NULL);
    dataAddress->time = *localtime(&t);
    char time[64];
    strftime(time, sizeof(time), "%c", &dataAddress->time);
    printf("Caracter Insertado: %c \nFecha: %s\nPosicion: %li\n",
           dataAddress->character, time, dataAddress-memoryAddress);
}

data* obtainNextDataAddress(data* currentDataAddress, int counter){
    if (counter%memorySize == 0){
        return memoryAddress;
    }else{
        return currentDataAddress + 1;
    }
}

void updateMetadataFinishedValue(int value){
        wait(metadataSemaphore);
        metadataStruct->clienteFinished = value;
        sem_post(metadataSemaphore);
}

void wait(sem_t* semaphore){
    struct timespec before, after;
    clock_gettime(CLOCK_REALTIME, &before);
    sem_wait(semaphore);
    clock_gettime(CLOCK_REALTIME, &after);
    secondsBlocked += after.tv_sec - before.tv_sec;
}