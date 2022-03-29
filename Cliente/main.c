#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <semaphore.h>
#include <time.h>
#include "../data.h"

long secondsBlocked;
long transferedCharacters;
const char* fileName = "./input.txt";
sem_t clientSemaphore;
sem_t reconstructorSemaphore;
sem_t metadataSemaphore;
int memSize;
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
    if (counter == memSize){
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
    memSize = 100;
    memoryAddress = (data*) malloc(memSize * sizeof(data));
    // Fix metadataAddress
    metadataAddress = (char*) malloc(memSize * sizeof(int));

    secondsBlocked = 0;
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
