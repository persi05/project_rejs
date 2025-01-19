#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include "shared.h"

int semid;
SharedData* shdata;

int isEndOfDay(int passenger_id) {
    P(semid, SEM_MUTEX);
    if (shdata->endOfDay == 1) {
        printf("\033[\033[1;34m[PASSENGER %d] Proces zakonczony. Koniec dnia: \033[0m", passenger_id);
        V(semid, SEM_MUTEX);
        return 1;
    }
    V(semid, SEM_MUTEX);
    return 0;
}

int main(int argc, char* argv[]) {
    if (argc != 2) {
        perror("\033[1;31mBledna liczba argumentow przy wywolaniu pasazer\033[0m\n");
    }

    int passenger_id = atoi(argv[1]);

    key_t semkey = ftok(".", SEM_PROJ_ID);
    if (semkey == -1) {
        perror("\033[1;31mBlad podczas generowania klucza semafora w pasazer\033[0m\n");
        exit(1);
    }
    semid = semget(semkey, NUM_SEMAPHORES, 0600);
    if (semid < 0) {
        perror("\033[1;31mBlad podczas otwierania semaforow w pasazer\033[0m\n");
        exit(1);
    }

    key_t shmkey = ftok(".", SHM_PROJ_ID);
    if (shmkey == -1) {
        perror("\033[1;31mBlad podczas generowania klucza pamieci dzielonej w pasazer\033[0m\n");
        exit(1);
    }
    int shmid = shmget(shmkey, sizeof(SharedData), 0600);
    if (shmid < 0) {
        perror("\033[1;31mBlad podczas otwierania pamieci dzielonej w pasazer\033[0m\n");
        exit(1);
    }

    shdata = (SharedData*) shmat(shmid, NULL, 0);
    if (shdata == (void*)-1) {
        perror("\033[1;31mBlad podczas laczenia z pamiecia dzielona w pasazer\033[0m\n");
        exit(1);
    }

    while (1) {
        if (isEndOfDay(passenger_id)) {
            if (shmdt(shdata) == -1) {
                perror("\033[1;31mBlad podczas odlaczania segmentu pamieci wspoldzielonej w pasazer\033[0m\n");
            }
        printf("\033[\033[1;34mKoniec rejsow.\033[0m\n");
        exit(0);
    }

    P(semid, SEM_MUTEX);
    int b = shdata->currentOnBridge;
    int s = shdata->currentOnShip;
    if (shdata->directionBridge == 0 && b < MOSTEK_POJ && shdata->isTrip == 0 && b + s < STATEK_POJ) {
        shdata->currentOnBridge++;
        printf("\033[0;33m[PASSENGER %d] wchodzi na mostek. Obecnie na mostku: %d, na statku: %d\033[0m\n",
                passenger_id, shdata->currentOnBridge, shdata->currentOnShip);
        V(semid, SEM_MUTEX);
        break;
    }

    V(semid, SEM_MUTEX);
   // usleep(100000);
    }

    //usleep(100000);

    while (1) {
        if (isEndOfDay(passenger_id)) {
            printf("\033[0;33mSchodzi do portu\n");
            if (shmdt(shdata) == -1) {
                perror("\033[1;31mBlad podczas odlaczania segmentu pamieci wspoldzielonej w pasazer\033[0m\n");
            }
            exit(0);
        }

        P(semid, SEM_MUTEX);
        if (shdata->directionBridge == 0 && shdata->currentOnShip < STATEK_POJ && shdata->isTrip == 0) {
            shdata->currentOnBridge--;
            shdata->currentOnShip++;
            printf("\033[0;33m[PASSENGER %d] Wszedl na statek. mostek: %d, statek: %d\033[0m\n",
                passenger_id, shdata->currentOnBridge, shdata->currentOnShip);
            V(semid, SEM_MUTEX);
            break;
        } else if (shdata->directionBridge == 1) {
            shdata->currentOnBridge--;
            printf("\033[0;33m[PASSENGER %d] Schodzi z mostku. mostek: %d\033[0m\n", passenger_id, shdata->currentOnBridge);
            V(semid, SEM_MUTEX);
            if (shmdt(shdata) == -1) {
                perror("\033[1;31mBlad podczas odlaczania segmentu pamieci wspoldzielonej w pasazer\033[0m\n");
            }
            exit(0);
        }
        V(semid, SEM_MUTEX);
        //usleep(100000);
    }
    //usleep(1000000);

    while (1) {
        if (isEndOfDay(passenger_id)) {
            printf("\033[0;33mSchodzi na mostek i do portu\033[0m\n");
            if (shmdt(shdata) == -1) {
            perror("\033[1;31mBlad podczas odlaczania segmentu pamieci wspoldzielonej w pasazer\033[0m\n");
        }
            return(0);
        }

        P(semid, SEM_MUTEX);
        if (shdata->directionBridge == 1 && shdata->currentOnBridge < MOSTEK_POJ && shdata->earlyTrip == 0 && shdata->isTrip == 0) {
            shdata->currentOnShip--;
            shdata->currentOnBridge++;
            printf("\033[0;33m[PASSENGER %d] Schodzi ze statku i wchodzi na mostek. mostek: %d, statek: %d\033[0m\n",
                passenger_id, shdata->currentOnBridge, shdata->currentOnShip);
            V(semid, SEM_MUTEX);
            break;
        }
        V(semid, SEM_MUTEX);
        usleep(300000);
    }
        //usleep(100000);
        P(semid, SEM_MUTEX);
        shdata->currentOnBridge--;
        printf("\033[0;33m[PASSENGER %d] Schodzi z mostku do portu. Koniec procesu.\033[0m\n", passenger_id);
        V(semid, SEM_MUTEX);

        if (shmdt(shdata) == -1) {
            perror("\033[1;31mBlad podczas odlaczania segmentu pamieci wspoldzielonej w pasazer\033[0m\n");
        }

        return 0;
    }
