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
                fprintf(stderr, "Niepoprawne argumenty - wszystkie argumenty musza byc dodatnie\n");
                exit(1);
        }
    if(MOSTEK_POJ >= STATEK_POJ) {
                fprintf(stderr, "Niepoprawne argumenty - pojemnosc mostku musi byc mniejsza niz statku\n");
                exit(1);
        }
    if(T2 >= T1) {
                fprintf(stderr, "Niepoprawne argumenty - T2 musi byc mniejsz niz T1\n");
                exit(1);
        }
}

int main() {
    arg_checker();
    
    key_t semkey = ftok(".", SEM_PROJ_ID);
    if (semkey == -1) {
        perror("Blad podczas generowania klucza w ftok()");
        exit(1);
    }
    key_t shmkey = ftok(".", SHM_PROJ_ID);
    if (shmkey == -1) {
        perror("Blad podczas generowania klucza w ftok()");
        exit(1);
    }

    int semid = create_semaphores(semkey);
    init_semaphore(semid, SEM_MUTEX, 1);
    printf("[MAIN] Semafory zainicjalizowane: SEM_MUTEX=1\n");

    int shmid = shmget(shmkey, sizeof(SharedData), IPC_CREAT | 0600);
    if (shmid < 0) {
        perror("Blad podczas tworzenia segmentu pamieci wspoldzielonej");
        exit(1);
    }

    SharedData* shdata = (SharedData*) shmat(shmid, NULL, 0);
    if (shdata == (void*)-1) {
        perror("Blad podczas przylaczania segmentu pamieci wspoldzielonej");
        exit(1);
    }

    shdata->currentOnBridge = 0;
    shdata->currentOnShip   = 0;
    shdata->totalRejsCount  = 0;
    shdata->isTrip          = 0;
    shdata->earlyTrip       = 0;
    shdata->endOfDay        = 0;
    shdata->directionBridge = 0;

    printf("[MAIN] Pamiec dzielona zainicjalizowana\n");

    pid_t pidKapitanStatku = fork();
    if (pidKapitanStatku == -1){
        perror("Blad podczas tworzenia procesu kapitan_statku");
        exit(1);
    }

    if (pidKapitanStatku == 0) {
        execl("./kapitan_statku", "kapitan_statku", NULL);
        perror("Blad podczas uruchomienia kapitan_statku (execl)");
        exit(1);
    }

    pid_t pidKapitanPortu = fork();
    if (pidKapitanPortu == -1){
        perror("Blad podczas tworzenia procesu kapitan_portu");
        exit(1);
    }
    if (pidKapitanPortu == 0) {
        char pidArg[10];
        snprintf(pidArg, sizeof(pidArg), "%d", pidKapitanStatku);
        execl("./kapitan_portu", "kapitan_portu", pidArg, NULL);
        perror("Blad podczas uruchamiania procesu kapitan_portu(execl)");
        exit(1);
    }

    srand(time(NULL));

    for (int i = 0; i < NUM_PASSENGERS; i++) {
        pid_t pidPassenger = fork();
        if (pidPassenger == -1) {
            perror("Blad podcazs odpalania pasazer.c w main");
            exit(1);
        }
        if (pidPassenger == 0) {
            char pArg[16];
            snprintf(pArg, sizeof(pArg), "%d", i + 1);
            execl("./pasazer", "pasazer", pArg, NULL);
            perror("Blad execl pasazer w main");
            exit(1);
        }
        //usleep(((rand() % 501) + 1500) * 100);
    }
    
    while (wait(NULL) > 0);

    remove_semaphores(semid);
    if (shmdt(shdata) == -1) {
        perror("Blad podczas odlaczania segmentu pamieci wspoldzielonej");
    }
    if (shmctl(shmid, IPC_RMID, NULL) == -1) {
        perror("Blad podczas usuwania segmentu pamieci wspoldzielonej");
    }

	return 0;
}