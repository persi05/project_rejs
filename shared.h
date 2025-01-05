#ifndef SHARED_H
#define SHARED_H

#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <sys/sem.h>

#define SEM_MUTEX         0
#define SEM_BRIDGE        1
#define SEM_SHIP          2 
#define NUM_SEMAPHORES    3

#define SHM_PROJ_ID      'A'
#define SEM_PROJ_ID      'B'

typedef struct {
    int currentOnBridge;
    int currentOnShip;
    int totalRejsCount;
    int earlyTrip;
    int endOfDay;
    int directionBridge;   //0 to w strone statku, 1 w strone portu
} SharedData;

int create_semaphores(key_t key);
void init_semaphore(int semid, int semnum, int value);
void P(int semid, int semnum);
void V(int semid, int semnum);
void remove_semaphores(int semid);
int get_semaphores(key_t key);

#endif