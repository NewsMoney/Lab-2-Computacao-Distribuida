#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/select.h>

#include "protocolo.h"

int ler_com_timeout(char *buffer, int tamanho, int segundos) {
    fd_set set;
    struct timeval timeout;

    FD_ZERO(&set);
    FD_SET(STDIN_FILENO, &set);

    timeout.tv_sec = segundos;
    timeout.tv_usec = 0;

    int rv = select(STDIN_FILENO + 1, &set, NULL, NULL, &timeout);

    if (rv == 0) return 0;
    if (rv < 0) return -1;

    fgets(buffer, tamanho, stdin);
    buffer[strcspn(buffer, "\n")] = 0;
    return 1;
}

int main(int argc, char *argv[]) {
    char *ip = (argc > 1) ? argv[1] : "127.0.0.1";
    int porta = (argc > 2) ? atoi(argv[2]) : PORTA_PADRAO;

    int sock = socket(AF_INET, SOCK_STREAM, 0);

    struct sockaddr_in serv;
    serv.sin_family = AF_INET;
    serv.sin_port = htons(porta);
    inet_pton(AF_INET, ip, &serv.sin_addr);

    connect(sock, (struct sockaddr*)&serv, sizeof(serv));

    char buffer[MAX_MSG];

    while (1) {
        int n = recv(sock, buffer, sizeof(buffer)-1, 0);
        if (n <= 0) break;

        buffer[n] = '\0';
        printf("%s", buffer);

        if (strncmp(buffer, "NOME|", 5) == 0) {
            char nome[50];
            fgets(nome, sizeof(nome), stdin);
            nome[strcspn(nome, "\n")] = 0;

            sprintf(buffer, "NOME|%s\n", nome);
            send(sock, buffer, strlen(buffer), 0);
        }

        if (strncmp(buffer, "RODADA|", 7) == 0) {
            char palavra[50];

            printf("Sua palavra (%ds): ", TEMPO_RODADA);

            int res = ler_com_timeout(palavra, sizeof(palavra), TEMPO_RODADA);

            if (res == 1) {
                sprintf(buffer, "PALAVRA|%s\n", palavra);
            } else {
                printf("\n⏱ Tempo esgotado!\n");
                sprintf(buffer, "TIMEOUT|\n");
            }

            send(sock, buffer, strlen(buffer), 0);
        }
    }

    close(sock);
    return 0;
}
