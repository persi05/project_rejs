#include "shared.h"

int create_semaphores(key_t key) {
    int semid = semget(key, NUM_SEMAPHORES, IPC_CREAT | 0600);
    if (semid < 0) {
        perror("\033[1;31mBlad przy tworzeniu zestawu semaforow (semget)\033[0m\n");
        exit(1);
    }
    return semid;
}

void init_semaphore(int semid, int semnum, int value) {
    if (semctl(semid, semnum, SETVAL, value) == -1) {
        perror("\033[1;31mBlad przy inicjalizacji semafora (semctl SETVAL)\033[0m\n");
        exit(1);
    }
}

void P(int semid, int semnum) {
    struct sembuf sb;
    sb.sem_num = semnum;
    sb.sem_op = -1;
    sb.sem_flg = 0;
    while (semop(semid, &sb, 1) == -1) {
    if (errno == EINTR) {
        fflush(stdout);
    } else {
        perror("\033[1;31mOperacja P nie powiodla sie (semop)\033[0m\n");
        exit(1);
    }
    }
}

void V(int semid, int semnum) {
    struct sembuf sb;
    sb.sem_num = semnum;
    sb.sem_op = +1; 
    sb.sem_flg = 0;
    while (semop(semid, &sb, 1) == -1) {
    if (errno == EINTR) {
        fflush(stdout);
    } else {
        perror("\033[1;31mOperacja V nie powiodla sie (semop)\033[0m\n");
        exit(1);
    }
    }
}

void remove_semaphores(int semid) {
    if (semctl(semid, 0, IPC_RMID) == -1) {
        perror("\033[1;31mBlad przy usuwaniu zestawu semaforow (semctl IPC_RMID)\033[0m\n");
    }
}