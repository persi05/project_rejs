#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <signal.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <time.h>
#include <errno.h> 
#include "shared.h"

int semid;
int shmid;
SharedData* shdata;
int a = 0;

volatile sig_atomic_t endOfDaySignal = 0;

void handle_signal(int sig) {
    if(sig == SIGUSR1) {
    P(semid, SEM_MUTEX);
    shdata->earlyTrip = 1;
    if(shdata->isTrip == 1) {
        shdata->endOfDay = 1;
        shdata->directionBridge = 1;
        V(semid, SEM_MUTEX);
        printf("Wyslano sig1 podczas rejsu - kataklizm wodny. Koncze procedure\n");
        exit(1);
    }
    V(semid, SEM_MUTEX);
    printf("[KAPITAN STATKU] Odebralem 'signal1'\n");
    }
    else if (sig == SIGUSR2){
    endOfDaySignal = 1;
    printf("[KAPITAN STATKU] Odebralem 'signal2' koniec dnia\n");
    }
}

void sail() {
    P(semid, SEM_MUTEX);
    shdata->totalRejsCount++;
    printf("[KAPITAN STATKU] Wyplywamy w rejs %d i jest %d pasazerow(czas lub sig1)\n", shdata->totalRejsCount, shdata->currentOnShip);
    V(semid, SEM_MUTEX);

    struct timespec req, rem;
    req.tv_sec = T2;
    req.tv_nsec = 0;

     while (nanosleep(&req, &rem) == -1) {
            if (errno == EINTR) {
                printf("[KAPITAN STATKU] Otrzymano sig2 podczas rejsu, wznawiam podroz\n");
                req = rem;
                a = 1;
            } 
            else {
                perror("nanosleep");
                exit(1);
            }
    }

    P(semid, SEM_MUTEX);
    shdata->isTrip = 0;
    V(semid, SEM_MUTEX);
}

void unload_passengers() {
    printf("[KAPITAN STATKU] Rozpoczynam wyladunek pasazerow-----\n");
    P(semid, SEM_MUTEX);
    shdata->directionBridge = 1;
    V(semid, SEM_MUTEX);
    while (1) {
        P(semid, SEM_MUTEX);
        int j = shdata->currentOnShip;
        int i = shdata->currentOnBridge;
        int k = shdata->earlyTrip;
        V(semid, SEM_MUTEX);
        fflush(0);
        if (j == 0 && i == 0 && k == 0) {
            printf("[KAPITAN STATKU] Nikogo nie ma na statku i mostku(wszyscy opuscili jesli byli)\n");
            break;
        }
        else if (k == 1) {
            printf("2");
            P(semid, SEM_MUTEX);
            shdata->isTrip = 1;
            shdata->directionBridge = 0;
            V(semid, SEM_MUTEX);
            sail();
            P(semid, SEM_MUTEX);
            shdata->directionBridge = 1;
            V(semid, SEM_MUTEX);
        }
        else if (a == 1) break;
    }
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
	sa.sa_handler = handle_signal;
	sigemptyset(&sa.sa_mask);
	sa.sa_flags = SA_RESTART;

    if (sigaction(SIGUSR1, &sa, NULL) == -1) {
        perror("blad sigaction signal1 w kapitan_statku\n");
        exit(1);
    }

    if (sigaction(SIGUSR2, &sa, NULL) == -1) {
        perror("blad sigaction signal2 w kapitan_statku\n");
        exit(1);
    }

    printf("[KAPITAN STATKU]-------START------\n");

    while (1) {
        int timeCount = 0;//chyba tak mala nieprecyzyjnosc bedzie ok
        printf("[KAPITAN STATKU] Rozpoczynam zaladunek pasazerow+++++\n");
        int waitTime = T1;
        while (timeCount < waitTime) {
            P(semid, SEM_MUTEX);
            shdata->directionBridge = 0;
            int earlyTrip = shdata->earlyTrip;
            int endOfDay = endOfDaySignal;
            V(semid, SEM_MUTEX);

            if (endOfDay) {
                printf("[KAPITAN STATKU] Sygnal SIGUSR2\n");
                P(semid, SEM_MUTEX);
                shdata->directionBridge = 1;
                V(semid, SEM_MUTEX);
                printf("[KAPITAN STATKU] Koniec procedury przez signal2\n");
                if (shmdt(shdata) == -1) {
                perror("Blad podczas odlaczania segmentu pamieci wspoldzielonej w kapitan_statku");
                exit(1);
                }
                return 0;
            }

            if (earlyTrip) {
                P(semid, SEM_MUTEX);
                shdata->earlyTrip = 0;
                shdata->isTrip = 1;
                V(semid, SEM_MUTEX);
                printf("[KAPITAN STATKU] Sygnal SIGUSR1 - odplywam\n");
                break;
            }
            timeCount++;
            printf("time: %d\n", timeCount);
            sleep(1);

        }

        P(semid, SEM_MUTEX);
        shdata->isTrip = 1;
        V(semid, SEM_MUTEX);

        sail();

        P(semid, SEM_MUTEX);
        shdata->endOfDay = a;
        V(semid, SEM_MUTEX);

        unload_passengers();

        P(semid, SEM_MUTEX);
        if (endOfDaySignal) {
            V(semid, SEM_MUTEX);
            printf("[KAPITAN STATKU] signal2 odebrany podczas rejsu, koniec procedury\n");
            if (shmdt(shdata) == -1) {
            perror("Blad podczas odlaczania segmentu pamieci wspoldzielonej w kapitan_statku");
            }
            return 0;
        }
        else if (shdata->totalRejsCount == MAXREJS) {
            shdata->endOfDay = 1;
            V(semid, SEM_MUTEX);
            printf("[KAPITAN STATKU] Maksymalna liczba rejsow (%d), koniec procedury\n", MAXREJS);
            if (shmdt(shdata) == -1) {
            perror("Blad podczas odlaczania segmentu pamieci wspoldzielonej w kapitan_statku");
            }
            return 0;
        }
        V(semid, SEM_MUTEX);
    }
}