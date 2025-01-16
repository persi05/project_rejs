#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <signal.h>
#include <string.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include "shared.h"

pid_t kapitanStatku_pid;

void send_signal1() {
    if (kill(kapitanStatku_pid, SIGUSR1) == -1) {
        perror("\033[1;31m[KAPITAN PORTU] Blad podczas wysylania 'signal1' do kapitan_statku\033[0m\n");
        exit(1);
    }
    printf("\033[\033[1;34m[KAPITAN PORTU] Wyslano 'signal1' (wczesniejsze wyplyniecie) do kapitan_statku\033[0m\n");
}

void send_signal2() {
    if (kill(kapitanStatku_pid, SIGUSR2) == -1) {
        perror("\033[1;31m[KAPITAN PORTU] Blad podczas wysylania 'signal2' do kapitan_statku\033[0m\n");
        exit(1);
    }
    printf("\033[\033[1;34m[KAPITAN PORTU] Wyslano 'signal2' (zakonczenie dnia) do kapitan_statku\033[0m\n");
}

void clear_buffer() {
    int a;
    while ((a = getchar()) != '\n' && a != EOF);
}

int main(int argc, char* argv[]) {
    if (argc != 2) {
        perror("\033[1;31mBledna liczba argumentow przy wywolaniu kapitan_portu\033[0m\n");
        exit(1);
    }

    kapitanStatku_pid = (pid_t)atoi(argv[1]);

    key_t semkey = ftok(".", SEM_PROJ_ID);
    if (semkey == -1) {
        perror("\033[1;31mBlad podczas generowania klucza semafora w kapitan_statku\033[0m\n");
        exit(1);
    }

    int semid = semget(semkey, NUM_SEMAPHORES, 0600);
    if (semid < 0) {
        perror("\033[1;31mBlad podczas otwierania semaforow w kapitan_statku\033[0m\n");
        exit(1);
    }

    printf("\033[\033[1;34m[KAPITAN PORTU]-------START------ wpisz 's' by wyslac signal1(odplyniecie), 'e' signal2(koniec dnia), 'k' by zakonczyc procedure\033[0m\n");

    char input;

    while (1) {
        input = getchar();
        while (getchar() != '\n');

        if (input == 's') {
            send_signal1();
        }
        else if (input == 'e') {
            send_signal2();
            printf("\033[0;32m[KAPITAN PORTU] Koniec dnia. Koniec procedury\033[0m\n");
            break;
        }
        else if (input == 'k') {
            printf("\033[0;32m[KAPITAN PORTU] Koniec procedury\033[0m\n");
            return 0;
        }
        else {
            printf("\033[0;32m[KAPITAN PORTU] Nieprawidlowy sygnal. Sprobuj ponownie.\033[0m\n");
        }
    }

    return 0;
}