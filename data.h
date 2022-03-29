#include <time.h>
#include <semaphore.h>

#ifndef PROYECTO_1_OPERATIVOS_DATA_H
#define PROYECTO_1_OPERATIVOS_DATA_H

#define MEM_SIZE 5
#define CLIENT_SEMAPHORE "client"
#define RECONSTRUCTOR_SEMAPHORE "reconstructor"
#define METADATA_SEMAPHORE "metadata"

typedef struct data{
    struct tm time;
    char character;
} data;

typedef struct sharedMemory{
    data sharedData[MEM_SIZE];
} sharedMemory;


#endif //PROYECTO_1_OPERATIVOS_DATA_H
