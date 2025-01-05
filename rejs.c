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
#define T1             1000   //czas co ile plynie statek (ms)
#define T2             3000   //czas trwania rejsu (ms)
#define MAX_REJS       3
#define NUM_PASSENGERS 50

typedef struct {
    int passenger_id;
    int semid;
    SharedData* shdata;
} PassengerArgs;



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
    init_semaphore(semid, SEM_SHIP, STATEK_POJ);
    printf("[DEBUG] Semafory zainicjalizowane: SEM_MUTEX=1, SEM_BRIDGE=%d, SEM_SHIP=%d\n", MOSTEK_POJ, STATEK_POJ);

    int shmid = shmget(shmkey, sizeof(SharedData), IPC_CREAT | 0600);
    if (shmid < 0) {
        perror("Blad podczas tworzenia segmentu pamieci wspoldzielonej");
        exit(1);
    }

    SharedData *shdata = (SharedData*) shmat(shmid, NULL, 0);
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

    printf("[DEBUG] Pamiec dzielona zainicjalizowana: maxRejs=%d, shipCapacity=%d, bridgeCapacity=%d\n",
       MAX_REJS, STATEK_POJ, MOSTEK_POJ);

    pid_t pidStatku = fork();
    if (pidStatku == -1){
        perror("Blad podczas tworzenia procesu kapitan_statku");
        exit(1);
    }

    if (pidStatku == 0) {
        //jakis char konwerter i execl 
         perror("Blad podczas uruchomienia kapitan_statku (execl)");
         exit(1);
    }

    /*
    ogarnac kapitan_statku i wyczyscic!!! na koncu i mozna raport jakis idk
    */

	return 0;
}