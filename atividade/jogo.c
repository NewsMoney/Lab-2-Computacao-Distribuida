#include <ctype.h>
#include <string.h>
#include <stdlib.h>
#include "jogo.h"

int validar_palavra(const char *palavra, char letra) {
    int len = strlen(palavra);

    if (len < 5) return 0;

    if (tolower(palavra[0]) != tolower(letra))
        return 0;

    for (int i = 0; i < len; i++) {
        if (!isalpha(palavra[i]))
            return 0;
    }

    return 1;
}

char gerar_letra() {
    return 'A' + rand() % 26;
}