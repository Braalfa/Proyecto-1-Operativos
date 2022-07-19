#include <time.h>
#include <semaphore.h>

#ifndef PROYECTO_1_OPERATIVOS_DATA_H
#define PROYECTO_1_OPERATIVOS_DATA_H

#define CLIENT_SEMAPHORE "client"
#define RECONSTRUCTOR_SEMAPHORE "reconstructor"
#define METADATA_SEMAPHORE "metadata"
#define FINALIZATION_SEMAPHORE "finalization"
#define MEMORY_KEY 0x1234
#define METADATA_KEY 0x1235

typedef struct data{
    struct tm time;
    char character;
} data;

typedef struct metaData{
    int clienteFinished;
    long clientBlockedSeconds;
    long reconstructorBlockedSeconds;
    long clientUserModeMicroSeconds;
    long reconstructorUserModeMicroSeconds;
    int transferedCharacters;
    int totalMemorySpace;
    int sharedMemorySize;
} metaData;

#endif //PROYECTO_1_OPERATIVOS_DATA_H
