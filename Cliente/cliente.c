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

#define MEMORY_KEY 0x1234

long secondsBlocked = 0;
long secondsUserMode;
long transferedCharacters;
const char* fileName = "./input.txt";
sem_t* clientSemaphore;
sem_t* reconstructorSemaphore;
sem_t* metadataSemaphore;
data *memoryAddress;
metaData *metadataStruct;
int textSize = 1000;

void loadSharedMemory();
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
    
    time_t start, end;
    time(&start);
    loadSharedMemory();
    loadSharedSemaphores();

    char text[textSize];
    if(readFile(text) == 0){
        return 0;
    }
    addTextToMemory(text);
    time(&end);

    secondsUserMode = end-start;
    transferedCharacters = strlen(text);
    addFinalMetadata();
    
    printf("\n\n");

    // Esto va en otro proceso
    sem_close(clientSemaphore);
    sem_close(reconstructorSemaphore);
    sem_close(metadataSemaphore);
    sem_unlink(CLIENT_SEMAPHORE);    
    sem_unlink(RECONSTRUCTOR_SEMAPHORE);    
    sem_unlink(METADATA_SEMAPHORE);    
    return 0;
}

void loadSharedMemory(){
    int shmid;
    struct shmseg *shmp;
    
    shmid = shmget(MEMORY_KEY, sizeof(MEM_SIZE * sizeof(data)), 0644|IPC_CREAT);
    if (shmid == -1) {
        perror("Shared memory");
    }

    // Attach to the segment to get a pointer to it.
    sharedMemory* sharedMem = shmat(shmid, NULL, 0);
    if (shmp == (void *) -1) {
        perror("Shared memory attach");
    }

    memoryAddress = sharedMem->sharedData;
    metadataStruct = &sharedMem->metaDataStruct;
}

void loadSharedSemaphores(){
    clientSemaphore = sem_open(CLIENT_SEMAPHORE, O_CREAT, 0644, MEM_SIZE);
    reconstructorSemaphore = sem_open(RECONSTRUCTOR_SEMAPHORE, O_CREAT, 0644, 0);
    metadataSemaphore = sem_open(METADATA_SEMAPHORE, O_CREAT, 0644, 1);
}

void addFinalMetadata(){
    wait(metadataSemaphore);
    metadataStruct->clientUserModeSeconds = secondsUserMode;
    metadataStruct->clientBlockedSeconds = secondsBlocked;
    metadataStruct->totalMemorySpace = sizeof(sharedMemory);
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
        printf("Pendiente: %s", text+counter);
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
    if (counter == MEM_SIZE){
        return memoryAddress;
    }else{
        return currentDataAddress + 1;
    }
}

void updateMetadataFinishedValue(int value){
        wait(metadataSemaphore);
        metadataStruct->finished = value;
        sem_post(metadataSemaphore);
}

void wait(sem_t* semaphore){
    struct timespec before, after;
    clock_gettime(CLOCK_REALTIME, &before);
    sem_wait(semaphore);
    clock_gettime(CLOCK_REALTIME, &after);
    secondsBlocked += after.tv_sec - before.tv_sec;
}