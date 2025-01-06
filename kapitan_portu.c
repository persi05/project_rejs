#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <signal.h>
#include <time.h>
//#include <sys/ipc.h>
//#include <sys/shm.h>
#include "shared.h"

pid_t kapitanStatku_pid;

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
    }

    srand(time(NULL));

    kapitanStatku_pid = (pid_t)atoi(argv[1]);

    printf("[KAPITAN PORTU]-------START------\n");

    /* mozliwe ze bede potrzebowal - musze pomyslec jak to zrobic - zakomentowane includy
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
    */

    while (1) {
        int random_time = (rand() % 5 + 8) * 1000000;
        usleep(random_time);
        send_signal1();

        clear_input_buffer();

        char input[10];

        if (fgets(input, sizeof(input), stdin) != NULL) {
            int length = strlen(input);
            if (length > 8) {
                printf("[KAPITAN PORTU] Wprowadzone dane do wyslania signal1 za dlugie. Wpisz tylko 's'\n");
                clear_buffer();
                continue;
            }
            if (strncmp(input, "s", 1) == 0) {
                send_signal2();
                break;
            }
        } else {
            printf("[KAPITAN PORTU] Blad podczas odczytu danych do signal1\n");
        }
    }

    /*
    if (shmdt(shdata) == -1) {
        perror("Blad podczas odlaczania segmentu pamieci wspoldzielonej w kapitan_portu");
    }
    */

    printf("[KAPITAN PORTU]------KONIEC------\n");
    return 0;
}
