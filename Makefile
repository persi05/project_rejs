CC = gcc
CFLAGS = -Wall -Wextra -pthread

all: rejs kapitan_statku kapitan_portu

rejs: rejs.c shared.c shared.h
	$(CC) $(CFLAGS) -o $@ rejs.c shared.c

kapitan_statku: kapitan_statku.c shared.c shared.h
	$(CC) $(CFLAGS) -o $@ kapitan_statku.c shared.c

kapitan_portu: kapitan_portu.c shared.c shared.h
	$(CC) $(CFLAGS) -o $@ kapitan_portu.c shared.c

clean:
	rm -f rejs kapitan_statku kapitan_portu
