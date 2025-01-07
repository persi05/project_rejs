#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/wait.h> 
#include "shared.h"

#define STATEK_POJ     5
#define MOSTEK_POJ     3
#define T1             3   //czas co ile plynie statek (s)
#define T2             1   //czas trwania rejsu (s)
#define MAX_REJS       3
#define NUM_PASSENGERS 50

typedef struct {
    int passenger_id;
    int semid;
} PassengerArgs;

SharedData *shdata;

void* passenger_thread(void* arg) {
    PassengerArgs* p = (PassengerArgs*) arg;

    while (1) {
        P(p->semid, SEM_MUTEX);
        if (shdata->directionBridge == 0) {
            if (shdata->currentOnBridge + shdata->currentOnShip < STATEK_POJ) {
                V(p->semid, SEM_MUTEX);
                break;
            }
        }

        V(p->semid, SEM_MUTEX);
        usleep(100000);
    }

    printf("[PASSENGER %d] Probuje wejsc na mostek===\n", p->passenger_id);

    P(p->semid, SEM_BRIDGE);

    P(p->semid, SEM_MUTEX);
    shdata->currentOnBridge++;
    printf("[PASSENGER %d] wchodzi na mostek+++. Obecnie na mostku: %d, na statku: %d\n",
           p->passenger_id, shdata->currentOnBridge, shdata->currentOnShip);
    V(p->semid, SEM_MUTEX);

    usleep(100000);

    while(1){
        P(p->semid, SEM_MUTEX);

        if (shdata->directionBridge == 0 && shdata->currentOnShip < STATEK_POJ) {
            shdata->currentOnBridge--;
            shdata->currentOnShip++;
            printf("[PASSENGER %d] Wszedl na statek+++. Obecnie na mostku: %d, na statku: %d\n",
                p->passenger_id, shdata->currentOnBridge, shdata->currentOnShip);
            V(p->semid, SEM_MUTEX);
            V(p->semid, SEM_BRIDGE);
            break;
        }

        V(p->semid, SEM_MUTEX);

        usleep(100000);
    }

    while (1) {
        P(p->semid, SEM_MUTEX);
        if (shdata->directionBridge == 1 && shdata->currentOnBridge < MOSTEK_POJ) {
            shdata->currentOnShip--;
            shdata->currentOnBridge++;
            printf("[PASSENGER %d] schodzi ze statku--- i wchodze na mostek. Obecnie na mostku: %d\n",
                   p->passenger_id, shdata->currentOnBridge);
            V(p->semid, SEM_MUTEX);
            P(p->semid, SEM_BRIDGE);
            break;
        }
        V(p->semid, SEM_MUTEX);
        usleep(100000);
    }

    P(p->semid, SEM_MUTEX);
    shdata->currentOnBridge--;
    printf("[PASSENGER %d] Zszedl z mostku do portu. Koniec watka. Obecnie na mostku: %d\n",
    p->passenger_id, shdata->currentOnBridge);
    V(p->semid, SEM_MUTEX);
    V(p->semid, SEM_BRIDGE);

    pthread_exit(NULL);
}

void arg_checker(){
    if (STATEK_POJ <= 0 || MOSTEK_POJ <= 0 || T1 <= 0 || T2 <= 0 || MAX_REJS <= 0) {
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
    init_semaphore(semid, SEM_BRIDGE, MOSTEK_POJ);
    printf("[MAIN] Semafory zainicjalizowane: SEM_MUTEX=1, SEM_BRIDGE, MOSTEK_POJ");

    int shmid = shmget(shmkey, sizeof(SharedData), IPC_CREAT | 0600);
    if (shmid < 0) {
        perror("Blad podczas tworzenia segmentu pamieci wspoldzielonej");
        exit(1);
    }

    shdata = (SharedData*) shmat(shmid, NULL, 0);
    if (shdata == (void*)-1) {
        perror("Blad podczas przylaczania segmentu pamieci wspoldzielonej");
        exit(1);
    }

    shdata->currentOnBridge = 0;
    shdata->currentOnShip   = 0;
    shdata->totalRejsCount  = 0;
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
        char maxRejsArg[10];
        char t1Arg[10];
        char t2Arg[10];
        snprintf(maxRejsArg, sizeof(maxRejsArg), "%d", MAX_REJS);
        snprintf(t1Arg, sizeof(t1Arg), "%d", T1);
        snprintf(t2Arg, sizeof(t2Arg), "%d", T2);
        execl("./kapitan_statku", "kapitan_statku", maxRejsArg, t1Arg, t2Arg, NULL);
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
        char maxRejsArg[10];
        snprintf(pidArg, sizeof(pidArg), "%d", pidKapitanStatku);
        snprintf(maxRejsArg, sizeof(maxRejsArg), "%d", MAX_REJS);
        execl("./kapitan_portu", "kapitan_portu", pidArg, maxRejsArg, NULL);
        perror("Blad podczas uruchamiania procesu kapitan_portu(execl)");
        exit(1);
    }   


   /*
   //jakos napisalbym tylko jeszcze zastanowic sie gdzie to dac i czy nie zmienic na dynamiczne
   //i czy moze po prostu caly czas ci sami goscie beda wchodzic i schodzic z mostku na rejs
    pthread_t passenger_threads[NUM_PASSENGERS];
    PassengerArgs pArgs[NUM_PASSENGERS];

     for (int i = 0; i < NUM_PASSENGERS; i++) {
        pArgs[i].passenger_id = i + 1;
        pArgs[i].semid = semid;

        if (pthread_create(&passenger_threads[i], NULL, passenger_thread, &pArgs[i]) != 0) {
            perror("Blad podczas tworzenia watku pasazera");
            exit(1);
        }
        //printf("[MAIN] Stworzono wątek pasażera nr %d\n", i + 1);
        
        }
        */

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