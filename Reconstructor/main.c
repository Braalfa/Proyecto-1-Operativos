#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <semaphore.h>
#include <time.h>
#include "../data.h"

long secondsBlocked;
long transferedCharacters;
sem_t* clientSemaphore;
sem_t* reconstructorSemaphore;
sem_t* metadataSemaphore;
int memSize;
data *memoryAddress;
char *metadataAddress;
int textSize = 1000;



data* obtainNextDataAddress(data* currentDataAddress, int counter){
    if (counter == memSize){
        return memoryAddress;
    }else{
        return currentDataAddress + 1;
    }
}

void loadCharFromDataAddress(char* text, data* dataAddress, int position){
    *(text+position) = dataAddress->character;
    *(text+position+1) = '\0';
    char time[64];
    strftime(time, sizeof(time), "%c", &dataAddress->time);
    printf("Caracter Obtenido: %c \nFecha de insercion: %s\nPosicion: %li\n",
           dataAddress->character, time, dataAddress-memoryAddress);
}

void wait(sem_t* semaphore){
    struct timespec before, after;
    clock_gettime(CLOCK_REALTIME, &before);
    sem_wait(semaphore);
    clock_gettime(CLOCK_REALTIME, &after);
    secondsBlocked = after.tv_sec - before.tv_sec;
}

void readTextFromMemory(){
    int counter = 0;
    data* currentDataAddress = memoryAddress;
    int finished = 0;
    char text[textSize];

    while (!finished){
        getchar();
        wait(&reconstructorSemaphore);
        loadCharFromDataAddress(text, currentDataAddress, counter);
        sem_post(&clientSemaphore);
        counter += 1;
        currentDataAddress = obtainNextDataAddress(currentDataAddress, counter);
        printf("Reconstruccion: %s", text);
    }
}

int main()
{
    time_t start, end;
    time(&start);
    memSize = 100;
    memoryAddress = (data *) 0x5555555592a0;
    // Fix metadataAddress
    metadataAddress = (char*) malloc(memSize * sizeof(int));

    secondsBlocked = 0;
    clientSemaphore = (sem_t*) 0x555555558060;
    reconstructorSemaphore = (sem_t*) 0x5555555580a0;
    metadataSemaphore = (sem_t*) 0x5555555580c0;

    readTextFromMemory();
    time(&end);

    long userModeTime = end-start;
    wait(&metadataSemaphore);
    // Escribir en metadata
    sem_post(&metadataSemaphore);
    return 0;
}
