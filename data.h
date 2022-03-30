#include <time.h>
#include <semaphore.h>

#ifndef PROYECTO_1_OPERATIVOS_DATA_H
#define PROYECTO_1_OPERATIVOS_DATA_H

#define MEM_SIZE 5
#define CLIENT_SEMAPHORE "client"
#define RECONSTRUCTOR_SEMAPHORE "reconstructor"
#define METADATA_SEMAPHORE "metadata"
#define MEMORY_KEY 0x1234

typedef struct data{
    struct tm time;
    char character;
} data;

typedef struct metaData{
    int finished;
    long clientBlockedSeconds;
    long reconstructorBlockedSeconds;
    long clientUserModeSeconds;
    long reconstructorUserModeSeconds;
    int transferedCharacters;
    int totalMemorySpace;
} metaData;

typedef struct sharedMemory{
    data sharedData[MEM_SIZE];
    metaData metaDataStruct;
} sharedMemory;

#endif //PROYECTO_1_OPERATIVOS_DATA_H
