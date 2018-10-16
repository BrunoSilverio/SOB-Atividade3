#include<stdio.h>
#include<stdlib.h>
#include<errno.h>
#include<fcntl.h>
#include<string.h>
#include<unistd.h>

#define BUFFER_LENGTH 256               ///< The buffer length (crude but fine)

static char receive[BUFFER_LENGTH];     ///< The receive buffer from the LKM

void stringtoHex(char *str);
void getString(char *str);
void readModule(char *str, int fd);
void writeModule(int fd);

int main(){
 int fd, opcao;
    char stringToSend[BUFFER_LENGTH];

    /*---------------------Open-file--------------------*/
    printf("Iniciando o modulo...\n");
    fd = open("/dev/cryptomodule", O_RDWR); // Abrir o arquivo com permissao para escrita e leitura
    if (fd < 0)
    {
        perror("Erro ao iniciar o modulo...");
        return errno;
    }

    // Menu
    printf("Menu:\n");
    printf("1- Encriptar String\n");
    printf("2- Encriptar Valores Hexadecimais\n");
    printf("3- Decriptar\n");
    printf("0- Sair\n");
    scanf("%d", &opcao);

    // Get string
    switch(opcao){
        case 1:
            stringtoHex(stringToSend);
            break;
        case 2:
        case 3:
            getString(stringToSend);
            break;
        case 0:
            return 0;
            break;
    }
    return 0;
    // Send String
    writeModule(stringToSend, fd);
    // Read Answer
    readModule(fd);

    return 0;
}
void stringtoHex(char *str){
    char strAux[BUFFER_LENGTH/2];
    int size = strlen(str);

    printf("String para codificar:\n");
    scanf("%s", str); // String que sera envida para o modulo
    printf("%s\n", str);
    int i;
    for(i = 0; i < strlen(str); i++){
        sprintf(&strAux[i*2], "%02X\n", str[i]);
    }
    str[i*2] = '\0';
    strcpy(str, strAux);
}
void getString(char *str){
    scanf("%s",str);
}
void readModule(char *str, int fd){
    int ret;
    printf("Escrevendo a mensagem [%s] no modulo .\n", str);
    ret = write(fd, str, strlen(str)); // envio da string para o modulo
    if (ret < 0)
    {
        perror("Erro no envio da string.");
        printf("Erro no: %ls", &errno);
        return;
    }
    printf("Aberte ENTER para ler a resposta..\n");
    getchar();

}

void writeModule(int fd){
    int ret;
    printf("Lendo do modulo...\n");
    ret = read(fd, receive, BUFFER_LENGTH); // Leitura do buffer
    if (ret < 0)
    {
        perror("Erro na leitura da mensagem.");
        printf("Erro no: %ls", &errno);
        return;
    }
    printf("Mensagem recebida: [%s]\n", receive);
    printf("Aberte ENTER para sair..\n");
    getchar();
}