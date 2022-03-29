#include <time.h>
#include <semaphore.h>

#ifndef PROYECTO_1_OPERATIVOS_DATA_H
#define PROYECTO_1_OPERATIVOS_DATA_H

#define MEM_SIZE 100
typedef struct data{
    struct tm time;
    char character;
} data;

typedef struct sharedMemory{
    data sharedData[MEM_SIZE];
    sem_t clientSemaphore;
    sem_t reconstructorSemaphore;
    sem_t metadataSemaphore;
} sharedMemory;


#endif //PROYECTO_1_OPERATIVOS_DATA_H
