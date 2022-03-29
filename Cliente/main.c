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

#define MEMORY_KEY 0x1234

long secondsBlocked;
long transferedCharacters;
const char* fileName = "./input.txt";
sem_t clientSemaphore;
sem_t reconstructorSemaphore;
sem_t metadataSemaphore;
data *memoryAddress;
char *metadataAddress;
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
    secondsBlocked = after.tv_sec - before.tv_sec;
}

void addTextToMemory(char* text){
    int counter = 0;
    data* currentDataAddress = memoryAddress;
    while (text[counter] != '\0'){
        getchar();
        wait(&clientSemaphore);
        writeCharToDataAddress(text[counter], currentDataAddress);
        sem_post(&reconstructorSemaphore);
        counter += 1;
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
    // Fix metadataAddress
    metadataAddress = (char*) malloc(MEM_SIZE * sizeof(int));

    secondsBlocked = 0;

    clientSemaphore = sharedMem->clientSemaphore;
    reconstructorSemaphore = sharedMem->reconstructorSemaphore;
    metadataSemaphore = sharedMem->metadataSemaphore;

    sem_init(&clientSemaphore, 0, 100);
    sem_init(&reconstructorSemaphore, 0, 0);
    sem_init(&metadataSemaphore, 0, 1);

    char text[textSize];
    readFile(text);
    addTextToMemory(text);
    time(&end);

    long userModeTime = end-start;
    wait(&metadataSemaphore);
    // Escribir en metadata
    sem_post(&metadataSemaphore);
    return 0;
}
