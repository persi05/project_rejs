#include "shared.h"

pid_t kapitanStatku_pid;

//funkcja odpowiedzialna za wysylanie sygnalu 1
void send_signal1() {
    if (kill(kapitanStatku_pid, SIGUSR1) == -1) {
        perror("\033[1;31m[KAPITAN PORTU] Blad podczas wysylania 'signal1' do kapitan_statku\033[0m\n");
        exit(1);
    }
    printf("\033[\033[1;34m[KAPITAN PORTU] Wyslano 'signal1' (wczesniejsze wyplyniecie) do kapitan_statku\033[0m\n");
}

//funkcja odpowiedzialna za wysylanie sygnalu 2
void send_signal2() {
    if (kill(kapitanStatku_pid, SIGUSR2) == -1) {
        perror("\033[1;31m[KAPITAN PORTU] Blad podczas wysylania 'signal2' do kapitan_statku\033[0m\n");
        exit(1);
    }
    printf("\033[\033[1;34m[KAPITAN PORTU] Wyslano 'signal2' (zakonczenie dnia) do kapitan_statku\033[0m\n");
}

//funkcja odpowiedzialna za czyszczenie bufforu
void clear_buffer() {
    int a;
    while ((a = getchar()) != '\n' && a != EOF);
}

int main() {
    int fifo_fd = open(fifo_path, O_RDONLY);
    if (fifo_fd == -1) {
        perror("\033[1;31m[KAPITAN PORTU] Blad otwarcia kolejki FIFO\033[0m\n");
        unlink(fifo_path);
        exit(1);
    }

    if (read(fifo_fd, &kapitanStatku_pid, sizeof(pid_t)) != sizeof(pid_t)) {
        perror("\033[1;31m[KAPITAN PORTU] Blad odczytu PID z kolejki FIFO\033[0m\n");
        close(fifo_fd);
        unlink(fifo_path);
        exit(1);
    }
    close(fifo_fd);

    printf("\033[\033[1;34m[KAPITAN PORTU] Otrzymano PID kapitana statku: %d\033[0m\n", kapitanStatku_pid);

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

    //funkcja odpowiedzialna za odczytywanie z klawiatury i wysyłanie odpowiednich sygnałów
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