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
        printf("[PASSENGER %d] Proces zakonczony. Koniec dnia: ", passenger_id);
        V(semid, SEM_MUTEX);
        return 1;
    }
    V(semid, SEM_MUTEX);
    return 0;
}

int main(int argc, char* argv[]) {
    if (argc != 2) {
        perror("Bledna liczba argumentow przy wywolaniu pasazer\n");
    }

    int passenger_id = atoi(argv[1]);

    key_t semkey = ftok(".", SEM_PROJ_ID);
    if (semkey == -1) {
        perror("Blad podczas generowania klucza semafora w pasazer");
        exit(1);
    }
    semid = semget(semkey, NUM_SEMAPHORES, 0600);
    if (semid < 0) {
        perror("Blad podczas otwierania semaforow w pasazer\n");
        exit(1);
    }

    key_t shmkey = ftok(".", SHM_PROJ_ID);
    if (shmkey == -1) {
        perror("Blad podczas generowania klucza pamieci dzielonej w pasazer\n");
        exit(1);
    }
    int shmid = shmget(shmkey, sizeof(SharedData), 0600);
    if (shmid < 0) {
        perror("Blad podczas otwierania pamieci dzielonej w pasazer\n");
        exit(1);
    }

    shdata = (SharedData*) shmat(shmid, NULL, 0);
    if (shdata == (void*)-1) {
        perror("Blad podczas laczenia z pamiecia dzielona w pasazer\n");
        exit(1);
    }

while (1) {
    if (isEndOfDay(passenger_id)) {
        if (shmdt(shdata) == -1) {
            perror("Blad podczas odlaczania segmentu pamieci wspoldzielonej w pasazer");
        }
        printf("Koniec rejsow.\n");
        exit(0);
    }

    P(semid, SEM_MUTEX);
    int b = shdata->currentOnBridge;
    int s = shdata->currentOnShip;
    if (shdata->directionBridge == 0 && b < MOSTEK_POJ && shdata->isTrip == 0 && b+s < STATEK_POJ) {
        shdata->currentOnBridge++;
        printf("[PASSENGER %d] wchodzi na mostek. Obecnie na mostku: %d, na statku: %d\n",
                passenger_id, shdata->currentOnBridge, shdata->currentOnShip);
        V(semid, SEM_MUTEX);
        break;
    }
    /*else {
        //printf("[PASSENGER %d] Nie moze wejsc na mostek, brak miejsca lub zly kierunek. Czekam mostek: %d, statek: %d\n",
        //      passenger_id, shdata->currentOnBridge, shdata->currentOnShip);
    }
    */
    V(semid, SEM_MUTEX);
    usleep(100000);
}

usleep(100000);

while (1) {
    if (isEndOfDay(passenger_id)) {
        printf("Schodzi do portu\n");
        if (shmdt(shdata) == -1) {
            perror("Blad podczas odlaczania segmentu pamieci wspoldzielonej w pasazer");
        }
        exit(0);
    }

    P(semid, SEM_MUTEX);
    if (shdata->directionBridge == 0 && shdata->currentOnShip < STATEK_POJ && shdata->isTrip == 0 && shdata->earlyTrip == 0) {
        shdata->currentOnBridge--;
        shdata->currentOnShip++;
        printf("[PASSENGER %d] Wszedl na statek. mostek: %d, statek: %d\n",
               passenger_id, shdata->currentOnBridge, shdata->currentOnShip);
        V(semid, SEM_MUTEX);
        break;
    } else if (shdata->directionBridge == 1) {// stad usunalem  i tam na dole isTrip
        shdata->currentOnBridge--;
        printf("[PASSENGER %d] Schodzi z mostku. mostek: %d\n", passenger_id, shdata->currentOnBridge);
        V(semid, SEM_MUTEX);
        shmdt(shdata);
        exit(0);
    }
    //mozna else dodac no bo wsm jesli zadne z powyzszych to sie juz nie zwolni miejsce na statku wiec mogą zejsc 
    V(semid, SEM_MUTEX);
    usleep(100000);
}

while (1) {
    if (isEndOfDay(passenger_id)) {
        printf("Schodzi na mostek i do portu\n");
        if (shmdt(shdata) == -1) {
        perror("Blad podczas odlaczania segmentu pamieci wspoldzielonej w pasazer");
    }
        return(0);
    }

    P(semid, SEM_MUTEX);
    if (shdata->directionBridge == 1 && shdata->currentOnBridge < MOSTEK_POJ && shdata->earlyTrip == 0 && shdata->isTrip == 0) {
        shdata->currentOnShip--;
        shdata->currentOnBridge++;
        printf("[PASSENGER %d] Schodzi ze statku i wchodzi na mostek. mostek: %d, statek: %d\n",
               passenger_id, shdata->currentOnBridge, shdata->currentOnShip);
        V(semid, SEM_MUTEX);
        break;
    }
    V(semid, SEM_MUTEX);
    usleep(300000);
}
    usleep(100000);
    P(semid, SEM_MUTEX);
    shdata->currentOnBridge--;
    printf("[PASSENGER %d] Schodzi z mostku do portu. Koniec procesu.\n", passenger_id);
    V(semid, SEM_MUTEX);

    if (shmdt(shdata) == -1) {
        perror("Blad podczas odlaczania segmentu pamieci wspoldzielonej w pasazer");
    }

    return 0;
}
