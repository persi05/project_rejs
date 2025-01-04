#ifndef SHARED_H
#define SHARED_H

#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <sys/sem.h>

#define SEM_MUTEX       0
#define SEM_BRIDGE      1
#define SEM_SHIP        2
#define NUM_SEMAPHORES  3

#define SHM_PATH        "."
#define SEM_PATH        "."
#define SHM_PROJ_ID     'S'
#define SEM_PROJ_ID     'M'

typedef struct {
    int currentOnBridge;
    int currentOnShip;
    int totalRejsCount;
    int endOfDay;
} SharedData;

#endif