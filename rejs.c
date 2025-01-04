#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include "shared.h"

#define STATEK_POJ 5
#define MOSTEK_POJ 3
#define T1         1000   //czas co ile plynie statek (ms)
#define T2         3000   //czas trwania rejsu (ms)
#define MAX_REJS   3

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
    
    key_t semkey = ftok(SEM_PATH, SEM_PROJ_ID);
    if (semkey == -1) {
        perror("Blad podczas generowania klucza w ftok()");
        exit(1);
    }
    key_t shmkey = ftok(SHM_PATH, SHM_PROJ_ID);
    if (shmkey == -1) {
        perror("Blad podczas generowania klucza w ftok()");
        exit(1);
    }
	return 0;
}