#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/wait.h> 
#include <time.h>
#include "shared.h"

void arg_checker(){
    if (STATEK_POJ <= 0 || MOSTEK_POJ <= 0 || T1 <= 0 || T2 <= 0 || MAXREJS <= 0) {
                fprintf(stderr, "\033[1;31mNiepoprawne argumenty - wszystkie argumenty musza byc dodatnie\033[0m\n");
                exit(1);
        }
    if(MOSTEK_POJ >= STATEK_POJ) {
                fprintf(stderr, "\033[1;31mNiepoprawne argumenty - pojemnosc mostku musi byc mniejsza niz statku\033[0m\n");
                exit(1);
        }
    if(T2 >= T1) {
                fprintf(stderr, "\033[1;31mNiepoprawne argumenty - T2 musi byc mniejszy niz T1\033[0m\n");
                exit(1);
        }
    if(NUM_PASSENGERS >= 400) {
                fprintf(stderr, "\033[1;31mNiepoprawne argumenty - liczba pasazerow musi byc mniejsza niz 400\033[0m\n");
                exit(1);
        }
}

int main() {
    arg_checker();
    
    key_t semkey = ftok(".", SEM_PROJ_ID);
    if (semkey == -1) {
        perror("\033[1;31mBlad podczas generowania klucza w ftok()\033[0m\n");
        exit(1);
    }
    key_t shmkey = ftok(".", SHM_PROJ_ID);
    if (shmkey == -1) {
        perror("\033[1;31mBlad podczas generowania klucza w ftok()\033[0m\n");
        exit(1);
    }

    int semid = create_semaphores(semkey);
    init_semaphore(semid, SEM_MUTEX, 1);
    printf("\033[\033[1;34m[MAIN] Semafory zainicjalizowane: SEM_MUTEX=1\033[0m\n");

    int shmid = shmget(shmkey, sizeof(SharedData), IPC_CREAT | 0600);
    if (shmid < 0) {
        perror("\033[1;31mBlad podczas tworzenia segmentu pamieci wspoldzielonej\033[0m\n");
        exit(1);
    }

    SharedData* shdata = (SharedData*) shmat(shmid, NULL, 0);
    if (shdata == (void*)-1) {
        perror("\033[1;31mBlad podczas przylaczania segmentu pamieci wspoldzielonej\033[0m\n");
        exit(1);
    }

    shdata->currentOnBridge = 0;
    shdata->currentOnShip   = 0;
    shdata->totalRejsCount  = 0;
    shdata->isTrip          = 0;
    shdata->earlyTrip       = 0;
    shdata->endOfDay        = 0;
    shdata->directionBridge = 0;

    printf("\033[\033[1;34m[MAIN] Pamiec dzielona zainicjalizowana\033[0m\n");

    pid_t pidKapitanStatku = fork();
    if (pidKapitanStatku == -1){
        perror("\033[1;31mBlad podczas tworzenia procesu kapitan_statku\033[0m\n");
        exit(1);
    }

    if (pidKapitanStatku == 0) {
        execl("./kapitan_statku", "kapitan_statku", NULL);
        perror("\033[1;31mBlad podczas uruchomienia kapitan_statku (execl)\033[0m\n");
        exit(1);
    }

    pid_t pidKapitanPortu = fork();
    if (pidKapitanPortu == -1){
        perror("\033[1;31mBlad podczas tworzenia procesu kapitan_portu\033[0m\n");
        exit(1);
    }
    if (pidKapitanPortu == 0) {
        char pidArg[10];
        snprintf(pidArg, sizeof(pidArg), "%d", pidKapitanStatku);
        execl("./kapitan_portu", "kapitan_portu", pidArg, NULL);
        perror("\033[1;31mBlad podczas uruchamiania procesu kapitan_portu(execl)\033[0m\n");
        exit(1);
    }

    srand(time(NULL));

    for (int i = 0; i < NUM_PASSENGERS; i++) {
        pid_t pidPassenger = fork();
        if (pidPassenger == -1) {
            perror("\033[1;31mBlad podczas odpalania pasazer.c w main\033[0m\n");
            exit(1);
        }
        if (pidPassenger == 0) {
            char pArg[16];
            snprintf(pArg, sizeof(pArg), "%d", i + 1);
            execl("./pasazer", "pasazer", pArg, NULL);
            perror("\033[1;31mBlad execl pasazer w main\033[0m\n");
            exit(1);
        }
        usleep(((rand() % 501) + 1500) * 100);
    }
    
    while (wait(NULL) > 0);

    remove_semaphores(semid);
    if (shmdt(shdata) == -1) {
        perror("\033[1;31mBlad podczas odlaczania segmentu pamieci wspoldzielonej\033[0m\n");
    }
    if (shmctl(shmid, IPC_RMID, NULL) == -1) {
        perror("\033[1;31mBlad podczas usuwania segmentu pamieci wspoldzielonej\033[0m\n");
    }

	return 0;
}