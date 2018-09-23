#include <stdio.h>
#include <stdlib.h>

#define TAMMAX 20


void shiftConcat(void const * const hex, size_t const size, void const * const newHex){
    unsigned char *b = (unsigned char*) hex;
    unsigned char *new = (unsigned char*) newHex;
    unsigned char byte;
    int i, j;
    int high = 1;

    j = 0;
    for(i = 0; i < size; i++){
        new[i] = (b[j] << 4) + b[j+1];
        j += 2;
    }

    for(i = 0; i < size/2; i++){
        for(j = 7; j >= 0; j--){
             byte = (new[i] >> j) & 1;
             printf("%u", byte);
        }
        printf(" ");
    }
    puts("");
}

int main(){

    unsigned char hex[TAMMAX];
    unsigned char newHex[TAMMAX/2];
    int i, j;
    char c;

    for(i = 0; i < TAMMAX; i++){
        c = getc(stdin);
        if(!( // Se c não pertencer '0' <= c <= '9' ou 'A' <= c <= 'F'
            (c >= '0' && c <= '9') ||
            (c >= 'A' && c <= 'F') ||
            c == EOF ||
            c == '\n'// '\n'
        )){ // caracter nao faz parte do conjunto permitido
            printf("%u Caracter Inválido!\n", c);
            return 0;
        }
        else{ // se fizer parte do conjunto permitido
            if(c == EOF || c == 0 || c == '\n'){ // Se chegar até o final do arquivo e não completou vetor, preencher com '0'
                for(j = i; j < TAMMAX; j++){
                    hex[j] = 0;
                }
                break;
            }
            else{ // Se não colocar caracter c no vetor;
                if(c <= '9')
                    hex[i] = c - 48;
                else
                    hex[i] = c - 55;
            }
        }
    }

    shiftConcat(hex, sizeof(hex), newHex);

    return 0;
}