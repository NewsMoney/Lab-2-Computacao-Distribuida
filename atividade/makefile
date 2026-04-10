CC = gcc
CFLAGS = -Wall -Wextra -pedantic -std=c11

all: servidor cliente

servidor:
	$(CC) $(CFLAGS) -o servidor servidor.c jogo.c -lpthread

cliente:
	$(CC) $(CFLAGS) -o cliente cliente.c jogo.c

clean:
	rm -f servidor cliente