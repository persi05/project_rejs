#ifndef SHARED_H
#define SHARED_H

#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <sys/sem.h>
#include <fcntl.h>
#include <string.h>
#include <sys/stat.h>
#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <errno.h>
#include <pthread.h>
#include <unistd.h>
#include <sys/wait.h>
#include <time.h>

#define SEM_MUTEX         0
#define NUM_SEMAPHORES    1

#define SHM_PROJ_ID      'A'
#define SEM_PROJ_ID      'B'

#define STATEK_POJ     20
#define MOSTEK_POJ     3
#define T1             5   //czas co ile plynie statek (s)
#define T2             3   //czas trwania rejsu (s)
#define MAXREJS        3
#define NUM_PASSENGERS MAXREJS*STATEK_POJ+MAXREJS*MOSTEK_POJ+5
#define fifo_path "/tmp/kapitan_fifo"

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