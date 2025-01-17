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
        printf("\033[1;31mWyslano sig1 podczas rejsu - kataklizm wodny. Koncze procedure\033[0m\n");
        if (shmdt(shdata) == -1) {
                perror("\033[1;31mBlad podczas odlaczania segmentu pamieci wspoldzielonej w kapitan_statku\033[0m\n");
                exit(1);
        }
        exit(1);
    }
    V(semid, SEM_MUTEX);
    printf("\033[0;35m[KAPITAN STATKU] Odebralem 'signal1'\033[0m\n");
    }
    else if (sig == SIGUSR2){
    endOfDaySignal = 1;
    printf("\033[0;35m[KAPITAN STATKU] Odebralem 'signal2' koniec dnia\033[0m\n");
    }
}

void perform_nanosleep(time_t seconds) {
    struct timespec req, rem;
    req.tv_sec = seconds;
    req.tv_nsec = 0;

    while (nanosleep(&req, &rem) == -1) {
        if (errno == EINTR) {
            printf("\033[0;37m[KAPITAN STATKU] Otrzymano sygnal podczas rejsu, wznawiam podroz\033[0m\n");
            req = rem;
            a = 1;
        } else {
            perror("\033[1;31m[Kapitan Statku] Blad z nanosleep\033[0m\n");
            if (shmdt(shdata) == -1) {
                perror("\033[1;31mBlad podczas odlaczania segmentu pamieci wspoldzielonej w kapitan_statku\033[0m\n");
                exit(1);
            }
            exit(1);
        }
    }
}

void sail() {
    P(semid, SEM_MUTEX);
    shdata->totalRejsCount++;
    shdata->isTrip = 1;
    shdata->directionBridge = 1;
    V(semid, SEM_MUTEX);

    while (1) {
        P(semid, SEM_MUTEX);
        if (shdata->currentOnBridge == 0) {
            V(semid, SEM_MUTEX);
            break;
        }
        V(semid, SEM_MUTEX);
    }

    P(semid, SEM_MUTEX);
    printf("\033[0;32m[KAPITAN STATKU] Wyplywamy w rejs %d i jest %d pasazerow(czas lub sig1)\033[0m\n", shdata->totalRejsCount, shdata->currentOnShip);
    V(semid, SEM_MUTEX);

    perform_nanosleep(T2);

    P(semid, SEM_MUTEX);
    shdata->isTrip = 0;
    V(semid, SEM_MUTEX);
}

void unload_passengers() {
    printf("\033[0;32m[KAPITAN STATKU] Rozpoczynam wyladunek pasazerow-----\033[0m\n");
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
            printf("\033[0;36m[KAPITAN STATKU] Nikogo nie ma na statku i mostku(wszyscy opuscili jesli byli)\033[0m\n");
            break;
        }
        else if (k == 1) {
            printf("2");
            P(semid, SEM_MUTEX);
            //shdata->directionBridge = 0;
            shdata->totalRejsCount++;
            shdata->isTrip = 1;
            V(semid, SEM_MUTEX);

            while (1) {
                P(semid, SEM_MUTEX);
                if (shdata->currentOnBridge == 0) {
                    V(semid, SEM_MUTEX);
                    break;
                }
                V(semid, SEM_MUTEX);
            }

            P(semid, SEM_MUTEX);
            printf("\033[0;32m[KAPITAN STATKU] Wyplywamy w rejs %d i jest %d pasazerow(czas lub sig1)\033[0m\n", shdata->totalRejsCount, shdata->currentOnShip);
            V(semid, SEM_MUTEX);

            perform_nanosleep(T2);
            
            P(semid, SEM_MUTEX);
            shdata->directionBridge = 1;
            shdata->earlyTrip = 0;
            shdata->isTrip = 0;
            V(semid, SEM_MUTEX);
        }
        else if (a == 1) break;
    }
}

int main() {
    key_t semkey = ftok(".", SEM_PROJ_ID);
    if (semkey == -1) {
        perror("\033[1;31mBlad podczas generowania klucza semafora w kapitan_statku\033[0m\n");
        exit(1);
    }

    semid = semget(semkey, NUM_SEMAPHORES, 0600);
    if (semid < 0) {
        perror("\033[1;31mBlad podczas otwierania semaforow w kapitan_statku\033[0m\n");
        exit(1);
    }

    key_t shmkey = ftok(".", SHM_PROJ_ID);
    if (shmkey == -1) {
        perror("\033[1;31mBlad podczas generowania klucza pamieci dzielonej w kapitan_statku\033[0m\n");
        exit(1);
    }

    int shmid = shmget(shmkey, sizeof(SharedData), 0600);
    if (shmid < 0) {
        perror("\033[1;31mBlad podczas otwierania pamieci dzielonej w kapitan_statku\033[0m\n");
        exit(1);
    }

    shdata = (SharedData*)shmat(shmid, NULL, 0);
    if (shdata == (void*)-1) {
        perror("\033[1;31mBlad podczas przylaczania segmentu pamieci dzielonej w kapitan_statku\033[0m\n");
        exit(1);
    }

    struct sigaction sa;
	sa.sa_handler = handle_signal;
	sigemptyset(&sa.sa_mask);
	sa.sa_flags = SA_RESTART;

    if (sigaction(SIGUSR1, &sa, NULL) == -1) {
        perror("\033[1;31mblad sigaction signal1 w kapitan_statku\033[0m\n");
        exit(1);
    }

    if (sigaction(SIGUSR2, &sa, NULL) == -1) {
        perror("\033[1;31mblad sigaction signal2 w kapitan_statku\033[0m\n");
        exit(1);
    }

    printf("\033[\033[1;34m[KAPITAN STATKU]-------START------\033[0m\n");

    while (1) {
        int timeCount = 0;//chyba tak mala nieprecyzyjnosc bedzie ok
        printf("\033[0;36m[KAPITAN STATKU] Rozpoczynam zaladunek pasazerow+++++\033[0m\n");
        int waitTime = T1;
        while (timeCount < waitTime) {
            P(semid, SEM_MUTEX);
            shdata->directionBridge = 0;
            int earlyTrip = shdata->earlyTrip;
            int endOfDay = endOfDaySignal;
            V(semid, SEM_MUTEX);

            if (endOfDay) {
                printf("\033[\033[1;34m[KAPITAN STATKU] Sygnal SIGUSR2\033[0m\n");
                P(semid, SEM_MUTEX);
                shdata->directionBridge = 1;
                shdata->endOfDay = 1;
                V(semid, SEM_MUTEX);
                printf("\033[\033[1;34m[KAPITAN STATKU] Koniec procedury przez signal2\033[0m\n");
                if (shmdt(shdata) == -1) {
                perror("\033[1;31mBlad podczas odlaczania segmentu pamieci wspoldzielonej w kapitan_statku\033[0m\n");
                exit(1);
                }
                return 0;
            }

            if (earlyTrip) {
                P(semid, SEM_MUTEX);
                shdata->earlyTrip = 0;
                V(semid, SEM_MUTEX);
                printf("\033[0;34m[KAPITAN STATKU] Sygnal SIGUSR1 - zara odplywam\033[0m\n");
                break;
            }
            timeCount++;
            //printf("time: %d\n", timeCount);
            sleep(1);

        }

        sail();

        P(semid, SEM_MUTEX);
        shdata->endOfDay = a;
        V(semid, SEM_MUTEX);

        unload_passengers();

        P(semid, SEM_MUTEX);
        if (endOfDaySignal) {
            V(semid, SEM_MUTEX);
            printf("\033[\033[1;34m[KAPITAN STATKU] signal2 odebrany podczas rejsu, koniec procedury\033[0m\n");
            if (shmdt(shdata) == -1) {
            perror("\033[1;31mBlad podczas odlaczania segmentu pamieci wspoldzielonej w kapitan_statku\033[0m\n");
            }
            return 0;
        }
        else if (shdata->totalRejsCount == MAXREJS) {
            shdata->endOfDay = 1;
            V(semid, SEM_MUTEX);
            printf("\033[\033[1;34m[KAPITAN STATKU] Maksymalna liczba rejsow (%d), koniec procedury\033[0m\n", MAXREJS);
            if (shmdt(shdata) == -1) {
            perror("\033[1;31mBlad podczas odlaczania segmentu pamieci wspoldzielonej w kapitan_statku\033[0m\n");
            }
            return 0;
        }
        V(semid, SEM_MUTEX);
    }
}