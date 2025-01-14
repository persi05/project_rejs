#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <signal.h>
#include <string.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include "shared.h"

pid_t kapitanStatku_pid;
SharedData* shdata;

void send_signal1() {
    if (kill(kapitanStatku_pid, SIGUSR1) == -1) {
        perror("[KAPITAN PORTU] Blad podczas wysylania 'signal1' do kapitan_statku\n");
        return;
    }
    printf("[KAPITAN PORTU] Wyslano 'signal1' (wczesniejsze wyplyniecie) do kapitan_statku\n");
}

void send_signal2() {
    if (kill(kapitanStatku_pid, SIGUSR2) == -1) {
        perror("[KAPITAN PORTU] Blad podczas wysylania 'signal2' do kapitan_statku\n");
        return;
    }
    printf("[KAPITAN PORTU] Wyslano 'signal2' (zakonczenie dnia) do kapitan_statku\n");
}

void clear_buffer() {
    int a;
    while ((a = getchar()) != '\n' && a != EOF);
}

int main(int argc, char* argv[]) {
    if (argc != 2) {
        perror("Bledna liczba argumentow przy wywolaniu kapitan_portu\n");
        exit(1);
    }

    kapitanStatku_pid = (pid_t)atoi(argv[1]);

    key_t semkey = ftok(".", SEM_PROJ_ID);
    if (semkey == -1) {
        perror("Blad podczas generowania klucza semafora w kapitan_statku\n");
        exit(1);
    }

    int semid = semget(semkey, NUM_SEMAPHORES, 0600);
    if (semid < 0) {
        perror("Blad podczas otwierania semaforow w kapitan_statku\n");
        exit(1);
    }

    key_t shmkey = ftok(".", SHM_PROJ_ID);
    if (shmkey == -1) {
        perror("Blad podczas generowania klucza w kapitan_port ftok()\n");
        exit(1);
    }

    int shmid = shmget(shmkey, sizeof(SharedData), 0600);
    if (shmid < 0) {
        perror("Blad podczas otwierania segmentu pamieci wspoldzielonej w kapitan_portu\n");
        exit(1);
    }

    shdata = (SharedData*)shmat(shmid, NULL, 0);
    if (shdata == (void*)-1) {
        perror("Blad podczas laczenia z pamiecia dzielona w kapitan_portu\n");
        exit(1);
    }

    printf("[KAPITAN PORTU]-------START------ wpisz 's' by wyslac signal1(odplyniecie), 'e' signal2(koniec dnia)\n");

    while (1) {
        P(semid, SEM_MUTEX);
        if(shdata->endOfDay == 1){
            V(semid, SEM_MUTEX);
            printf("[KAPITAN PORTU] Maksymalna liczba rejsow (%d) lub koniec dnia, koniec procedury\n", MAXREJS);
            if (shmdt(shdata) == -1) {
            perror("Blad podczas odlaczania segmentu pamieci wspoldzielonej w kapitan_portu");
            }
            return 0;
        }
        V(semid, SEM_MUTEX);
/*
        char input[10];

        if (fgets(input, sizeof(input), stdin) != NULL) {
            int length = strlen(input);
            if (length > 2) {
                printf("[KAPITAN PORTU] Wprowadzone dane do wyslania signal1 za dlugie. Wpisz tylko 's' lub 'e'\n");
            }
            else if (strncmp(input, "s", 1) == 0) {
                send_signal1();
                printf("[KAPITAN PORTU] Odczytuje i wysylam sygnal 1 (wczesniejesze odplyniecie)\n");
            }
            else if (strncmp(input, "e", 1) == 0) {
                send_signal2();
                printf("[KAPITAN PORTU] Odczytuje i wysylam sygnal 2 (koniec dnia)");
                printf("[Kapitan PORTU] Koniec dnia. Koniec procedury");
                break;
            }
            else {
                printf("[KAPITAN PORTU] Nieprawidlowa komenda. Wpisz 's' lub 'e'\n");
            } 
        }
        else {
            printf("[KAPITAN PORTU] Blad podczas odczytu danych do signal1\n");
        }
*/
    }

    if (shmdt(shdata) == -1) {
        perror("Blad podczas odlaczania segmentu pamieci wspoldzielonej w kapitan_portu\n");
    }
    return 0;
}