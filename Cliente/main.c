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

long secondsBlocked;
long transferedCharacters;
const char* fileName = "./input.txt";
sem_t* clientSemaphore;
sem_t* reconstructorSemaphore;
sem_t* metadataSemaphore;
data *memoryAddress;
metaData *metadataStruct;
int textSize = 1000;


void readFile(char* text)
{
    int counter = 0;
    FILE *file = fopen(fileName, "r");
    int character;
    if (file == NULL){
        printf("No se encontró el archivo de lectura");
        return;
    }
    while ((character = fgetc(file)) != EOF)
    {
        text[counter] = (char) character;
        counter += 1;
    }
    text[counter] = '\0';
}

data* obtainNextDataAddress(data* currentDataAddress, int counter){
    if (counter == MEM_SIZE){
        return memoryAddress;
    }else{
        return currentDataAddress + 1;
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

void wait(sem_t* semaphore){
    struct timespec before, after;
    clock_gettime(CLOCK_REALTIME, &before);
    sem_wait(semaphore);
    clock_gettime(CLOCK_REALTIME, &after);
    secondsBlocked += after.tv_sec - before.tv_sec;
}

void updateMetadataFinishedValue(int value){
        wait(metadataSemaphore);
        metadataStruct->finished = value;
        sem_post(metadataSemaphore);
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

int main()
{
    time_t start, end;
    time(&start);

    int shmid, numtimes;
    struct shmseg *shmp;
    char *bufptr;
    int spaceavailable;
    
    shmid = shmget(MEMORY_KEY, sizeof(MEM_SIZE * sizeof(data)), 0644|IPC_CREAT);
    if (shmid == -1) {
        perror("Shared memory");
        return 1;
    }

    // Attach to the segment to get a pointer to it.
    sharedMemory* sharedMem = shmat(shmid, NULL, 0);
    if (shmp == (void *) -1) {
        perror("Shared memory attach");
        return 1;
    }

    memoryAddress = sharedMem->sharedData;
    metadataStruct = &sharedMem->metaDataStruct;

    secondsBlocked = 0;

    clientSemaphore = sem_open(CLIENT_SEMAPHORE, O_CREAT, 0644, MEM_SIZE);
    reconstructorSemaphore = sem_open(RECONSTRUCTOR_SEMAPHORE, O_CREAT, 0644, 0);
    metadataSemaphore = sem_open(METADATA_SEMAPHORE, O_CREAT, 0644, 1);

    char text[textSize];
    readFile(text);
    addTextToMemory(text);
    time(&end);

    long secondsUserMode = end-start;
    wait(metadataSemaphore);
    metadataStruct->clientUserModeSeconds = secondsUserMode;
    metadataStruct->clientBlockedSeconds = secondsBlocked;
    metadataStruct->totalMemorySpace = sizeof(sharedMemory);
    metadataStruct->transferedCharacters = strlen(text);
    sem_post(metadataSemaphore);

    sem_close(clientSemaphore);
    sem_close(reconstructorSemaphore);
    sem_close(metadataSemaphore);
    sem_unlink(CLIENT_SEMAPHORE);    
    sem_unlink(RECONSTRUCTOR_SEMAPHORE);    
    sem_unlink(METADATA_SEMAPHORE);    
    return 0;
}
