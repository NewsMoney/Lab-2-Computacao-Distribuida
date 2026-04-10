#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <pthread.h>
#include <arpa/inet.h>
#include <time.h>
#include <stdarg.h>

#include "protocolo.h"
#include "jogo.h"

typedef struct {
    int c1;
    int c2;
} partida_t;

void enviar(int sock, const char *fmt, ...) {
    char buffer[MAX_MSG];
    va_list args;
    va_start(args, fmt);
    vsnprintf(buffer, MAX_MSG, fmt, args);
    va_end(args);

    send(sock, buffer, strlen(buffer), 0);
}

int receber_palavra(int sock, char *palavra) {
    char buffer[MAX_MSG] = {0};
    recv(sock, buffer, sizeof(buffer), 0);

    if (strncmp(buffer, "PALAVRA|", 8) == 0) {
        sscanf(buffer, "PALAVRA|%s", palavra);
        return 1;
    }

    if (strncmp(buffer, "TIMEOUT|", 8) == 0) {
        strcpy(palavra, "");
        return 0;
    }

    return 0;
}

void *thread_partida(void *arg) {
    partida_t *p = (partida_t *)arg;

    char nome1[MAX_NOME], nome2[MAX_NOME];
    char buffer[MAX_MSG];

    enviar(p->c1, "NOME|\n");
    recv(p->c1, buffer, sizeof(buffer), 0);
    sscanf(buffer, "NOME|%s", nome1);

    enviar(p->c1, "AGUARDE|Esperando outro jogador...\n");

    enviar(p->c2, "NOME|\n");
    recv(p->c2, buffer, sizeof(buffer), 0);
    sscanf(buffer, "NOME|%s", nome2);

    enviar(p->c1, "MSG|%s vs %s\n", nome1, nome2);
    enviar(p->c2, "MSG|%s vs %s\n", nome1, nome2);

    int pts1 = 0, pts2 = 0;

    for (int r = 1; r <= RODADAS; r++) {
        char letra = gerar_letra();

        enviar(p->c1, "RODADA|%d|%c|%d\n", r, letra, TEMPO_RODADA);
        enviar(p->c2, "RODADA|%d|%c|%d\n", r, letra, TEMPO_RODADA);

        char w1[50] = "", w2[50] = "";

        int r1 = receber_palavra(p->c1, w1);
        int r2 = receber_palavra(p->c2, w2);

        int v1 = r1 && validar_palavra(w1, letra);
        int v2 = r2 && validar_palavra(w2, letra);

        int repetida = strcmp(w1, w2) == 0 && strlen(w1) > 0;

        if (v1 && !repetida) pts1++;
        if (v2 && !repetida) pts2++;

        enviar(p->c1, "RESULTADO|%s (%s)\n", w1, v1 ? "OK" : "INVÁLIDA");
        enviar(p->c2, "RESULTADO|%s (%s)\n", w2, v2 ? "OK" : "INVÁLIDA");

        enviar(p->c1, "PLACAR|%s|%d|%s|%d\n", nome1, pts1, nome2, pts2);
        enviar(p->c2, "PLACAR|%s|%d|%s|%d\n", nome1, pts1, nome2, pts2);
    }

    char resultado[100];
    if (pts1 > pts2)
        sprintf(resultado, "%s venceu!", nome1);
    else if (pts2 > pts1)
        sprintf(resultado, "%s venceu!", nome2);
    else
        sprintf(resultado, "Empate!");

    enviar(p->c1, "FIM|%s\n", resultado);
    enviar(p->c2, "FIM|%s\n", resultado);

    close(p->c1);
    close(p->c2);
    free(p);
    return NULL;
}

int main(int argc, char *argv[]) {
    srand(time(NULL));

    int porta = (argc > 1) ? atoi(argv[1]) : PORTA_PADRAO;

    int server = socket(AF_INET, SOCK_STREAM, 0);

    struct sockaddr_in serv;
    serv.sin_family = AF_INET;
    serv.sin_port = htons(porta);
    serv.sin_addr.s_addr = INADDR_ANY;

    bind(server, (struct sockaddr*)&serv, sizeof(serv));
    listen(server, 10);

    printf("Servidor rodando na porta %d\n", porta);

    while (1) {
        int c1 = accept(server, NULL, NULL);
        printf("Jogador 1 conectado\n");

        int c2 = accept(server, NULL, NULL);
        printf("Jogador 2 conectado\n");

        partida_t *p = malloc(sizeof(partida_t));
        p->c1 = c1;
        p->c2 = c2;

        pthread_t t;
        pthread_create(&t, NULL, thread_partida, p);
        pthread_detach(t);
    }
}