#ifndef SHARED_H
#define SHARED_H

#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <sys/sem.h>

#define SEM_MUTEX         0
#define NUM_SEMAPHORES    1

#define SHM_PROJ_ID      'A'
#define SEM_PROJ_ID      'B'

#define STATEK_POJ     10
#define MOSTEK_POJ     6
#define T1             3   //czas co ile plynie statek (s)
#define T2             1   //czas trwania rejsu (s)
#define MAXREJS        5
#define NUM_PASSENGERS MAXREJS*STATEK_POJ+MAXREJS*MOSTEK_POJ+5

typedef struct {
    int currentOnBridge;
    int currentOnShip;
    int totalRejsCount;
    int earlyTrip;
    int isTrip;
    int endOfDay;
    int directionBridge;   //0 to w strone statku, 1 w strone portu
} SharedData;

int create_semaphores(key_t key);
void init_semaphore(int semid, int semnum, int value);
void P(int semid, int semnum);
void V(int semid, int semnum);
void remove_semaphores(int semid);

#endif