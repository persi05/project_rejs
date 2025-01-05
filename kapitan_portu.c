#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include "shared.h"

int main() {
    key_t shmkey = ftok(".", SHM_PROJ_ID);
    if (shmkey == -1) {
        perror("Blad podczas generowania klucza w kapitan_port ftok()");
        exit(1);
    }

    int shmid = shmget(shmkey, sizeof(SharedData), 0600);
    if (shmid < 0) {
        perror("Blad podczas otwierania segmentu pamieci wspoldzielonej w kapitan_portu");
        exit(1);
    }

    SharedData* shdata = (SharedData*)shmat(shmid, NULL, 0);
    if (shdata == (void*)-1) {
        perror("Blad podczas przylaczania segmentu pamieci wspoldzielonej w kapitan_portu");
        exit(1);
    }

    printf("Startuje procedure kapitan_portu\n");

    while (1) {
        //trzeba napisac to przechwytywanie z klawiatury tych sygnalow i wysylanie
    }

    if (shmdt(shdata) == -1) {
        perror("Blad podczas odlaczania segmentu pamieci wspoldzielonej w kapitan_portu");
    }

    printf("Procedura kapitan_portu zakonczona\n");
    return 0;
}
