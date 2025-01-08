#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <signal.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <time.h>
#include "shared.h"

int semid;
int shmid;
SharedData* shdata;

void handle_signal1(int sig) {
    P(semid, SEM_MUTEX);
    shdata->earlyTrip = 1;
    V(semid, SEM_MUTEX);
    printf("[KAPITAN STATKU] 'signal1' wyplywamy wczesniej\n");
}

void handle_signal2(int sig) {
    P(semid, SEM_MUTEX);
    shdata->endOfDay = 1;
    V(semid, SEM_MUTEX);
    printf("[KAPITAN STATKU] 'signal2' koniec dnia\n");
}

void unload_passengers() {
    printf("[KAPITAN STATKU] Rozpoczynam wyladunek pasazerow-----\n");
    P(semid, SEM_MUTEX);
    shdata->directionBridge = 1;
    V(semid, SEM_MUTEX);
    //printf("tutaj1");

    while (1) {
        P(semid, SEM_MUTEX);
       // printf("tutaj4");
        printf("currentOnShip1: %d, currentOnBridge1: %d\n", shdata->currentOnShip, shdata->currentOnBridge);
        
        if (shdata->currentOnShip == 0 && shdata->currentOnBridge == 0) {
            printf("currentOnShip2: %d, currentOnBridge2: %d\n", shdata->currentOnShip, shdata->currentOnBridge);

           // printf("tutaj2");
            printf("[KAPITAN STATKU] Wszyscy pasazerowie opuscili statek i most\n");
            V(semid, SEM_MUTEX);
            break;
        }
        
        V(semid, SEM_MUTEX);
        usleep(500000);
    }
}

void load_passengers() {
    printf("[KAPITAN STATKU] Rozpoczynam zaladunek pasazerow+++++\n");
    P(semid, SEM_MUTEX);
    shdata->directionBridge = 0;
    V(semid, SEM_MUTEX);

    while (1) {
        P(semid, SEM_MUTEX);
        if (shdata->currentOnShip < STATEK_POJ) {
            printf("[KAPITAN STATKU] Wszyscy pasażerowie weszli na statek\n");
            V(semid, SEM_MUTEX);
            break;
        }
        V(semid, SEM_MUTEX);
        usleep(100000);
    }
}

void sail() {
    P(semid, SEM_MUTEX);
    shdata->totalRejsCount++;
    printf("[KAPITAN STATKU] Wyplywamy w rejs %d i jest %d pasazerow\n", shdata->totalRejsCount, shdata->currentOnShip);
    V(semid, SEM_MUTEX);

    sleep(T2);

    unload_passengers();
}

int main() {
    key_t semkey = ftok(".", SEM_PROJ_ID);
    if (semkey == -1) {
        perror("Blad podczas generowania klucza semafora w kapitan_statku\n");
        exit(1);
    }

    semid = semget(semkey, NUM_SEMAPHORES, 0600);
    if (semid < 0) {
        perror("Blad podczas otwierania semaforow w kapitan_statku\n");
        exit(1);
    }

    key_t shmkey = ftok(".", SHM_PROJ_ID);
    if (shmkey == -1) {
        perror("Blad podczas generowania klucza pamieci dzielonej w kapitan_statku\n");
        exit(1);
    }

    int shmid = shmget(shmkey, sizeof(SharedData), 0600);
    if (shmid < 0) {
        perror("Blad podczas otwierania pamieci dzielonej w kapitan_statku\n");
        exit(1);
    }

    shdata = (SharedData*)shmat(shmid, NULL, 0);
    if (shdata == (void*)-1) {
        perror("Blad podczas przylaczania segmentu pamieci dzielonej w kapitan_statku\n");
        exit(1);
    }

    struct sigaction sa;
	sa.sa_handler = handle_signal1;
	sigemptyset(&sa.sa_mask);
	sa.sa_flags = 0;

    if (sigaction(SIGUSR1, &sa, NULL) == -1) {
        perror("blad sigaction signal1 w kapitan_statku\n");
        exit(1);
    }

    sa.sa_handler = handle_signal2;

    if (sigaction(SIGUSR2, &sa, NULL) == -1) {
        perror("blad sigaction signal2 w kapitan_statku\n");
        exit(1);
    }

    printf("[KAPITAN STATKU]-------START------\n");

    while (1) {
        load_passengers();

        int timeCount = 0;//chyba tak mala nieprecyzyjnosc bedzie ok

        while (timeCount < T1) {
            P(semid, SEM_MUTEX);
            int earlyTrip = shdata->earlyTrip;
            int endOfDay = shdata->endOfDay;
            V(semid, SEM_MUTEX);

            if (endOfDay) {
                printf("[KAPITAN STATKU] Sygnal SIGUSR2\n");
                unload_passengers();
                if (shmdt(shdata) == -1) {
                perror("Blad podczas odlaczania segmentu pamieci wspoldzielonej w kapitan_statku");
                }
                return 0;
            }

            if (earlyTrip) {
                P(semid, SEM_MUTEX);
                shdata->earlyTrip = 0;
                V(semid, SEM_MUTEX);
                printf("[KAPITAN STATKU] Sygnal SIGUSR1 - odplywam\n");
                break;
            }

            sleep(1);
            timeCount++;
        }

        P(semid, SEM_MUTEX);
        if (shdata->totalRejsCount >= MAXREJS) {
            V(semid, SEM_MUTEX);
            printf("[KAPITAN STATKU] Maksymalna liczba rejsow (%d), koniec procedury\n", MAXREJS);
            if (shmdt(shdata) == -1) {
            perror("Blad podczas odlaczania segmentu pamieci wspoldzielonej w kapitan_statku");
            }
            return 0;
        }
        V(semid, SEM_MUTEX);

        sail();

        P(semid, SEM_MUTEX);
        if (shdata->endOfDay) {
            V(semid, SEM_MUTEX);
            printf("[KAPITAN STATKU] Koniec, signal2 odebrany podczas rejsu\n");
            if (shmdt(shdata) == -1) {
            perror("Blad podczas odlaczania segmentu pamieci wspoldzielonej w kapitan_statku");
            }
            return 0;
        }
        V(semid, SEM_MUTEX);
    }

    if (shmdt(shdata) == -1) {
        perror("Blad podczas odlaczania segmentu pamieci wspoldzielonej w kapitan_statku");
    }
    return 0;
}