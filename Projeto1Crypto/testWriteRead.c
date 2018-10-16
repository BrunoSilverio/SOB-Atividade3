#include<stdio.h>
#include<stdlib.h>
#include<errno.h>
#include<fcntl.h>
#include<string.h>
#include<unistd.h>

#define BUFFER_LENGTH 256               ///< The buffer length (crude but fine)

static char receive[BUFFER_LENGTH];     ///< The receive buffer from the LKM
 
int main(){
 int ret, fd, opcao;
    char stringToSend[BUFFER_LENGTH];

    /*---------------------Open-file--------------------*/
    printf("Iniciando o modulo...\n");
    fd = open("/dev/cryptomodule", O_RDWR); // Abrir o arquivo com permissao para escrita e leitura
    if (fd < 0)
    {
        perror("Erro ao iniciar o modulo...");
        return errno;
    }

    /*---------------------Write------------------------*/
    printf("Digite a string que sera enviada:\n");
    scanf("%[^\n]%*c", stringToSend); // String que sera envida para o modulo
    printf("Escrevendo a mensagem [%s] no modulo .\n", stringToSend);
    ret = write(fd, stringToSend, strlen(stringToSend)); // envio da string para o modulo
    if (ret < 0)
    {
        perror("Erro no envio da string.");
        return errno;
    }
    printf("Aberte ENTER para ler a resposta..\n");
    getchar();

    /*---------------------Read------------------------*/
    printf("Lendo do modulo...\n");
    ret = read(fd, receive, BUFFER_LENGTH); // Leitura do buffer
    if (ret < 0)
    {
        perror("Erro na leitura da mensagem.");
        return errno;
    }
    printf("Mensagem recebida: [%s]\n", receive);
    printf("Aberte ENTER para sair..\n");
    getchar();

    return 0;
}