#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <signal.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include "shared.h"

int semid;
SharedData* shdata;

int main(){
    key_t semkey = ftok(".", SEM_PROJ_ID);
    if (semkey == -1) {
        perror("Blad podczas generowania klucza semafora w kapitan_statku");
        exit(1);
    }

    semid = semget(semkey, NUM_SEMAPHORES, 0600);
    if (semid < 0) {
        perror("Blad podczas otwierania semaforow w kapitan_statku");
        exit(1);
    }

    key_t shmkey = ftok(".", SHM_PROJ_ID);
    if (shmkey == -1) {
        perror("Blad podczas generowania klucza pamieci dzielonej w kapitan_statku");
        exit(1);
    }

    int shmid = shmget(shmkey, sizeof(SharedData), 0600);
    if (shmid < 0) {
        perror("Blad podczas otwierania pamieci dzielonej w kapitan_statku");
        exit(1);
    }

    shdata = (SharedData*)shmat(shmid, NULL, 0);
    if (shdata == (void*)-1) {
        perror("Blad podczas przylaczania segmentu pamieci dzielonej w kapitan_statku");
        exit(1);
    }    
}