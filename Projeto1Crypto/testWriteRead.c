/**
 * @file   testebbchar.c
 * @author Derek Molloy
 * @date   7 April 2015
 * @version 0.1
 * @brief  A Linux user space program that communicates with the ebbchar.c LKM. It passes a
 * string to the LKM and reads the response from the LKM. For this example to work the device
 * must be called /dev/ebbchar.
 * @see http://www.derekmolloy.ie/ for a full description and follow-up descriptions.
*/
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <fcntl.h>
#include <string.h>
#include <unistd.h>

#define BUFFER_LENGTH 256           //Tamanho do buffer
static char receive[BUFFER_LENGTH]; //buffer que ira receber a mensagem

int main()
{
    int ret, fd;
    char stringToSend[BUFFER_LENGTH];
    printf("Iniciando o modulo...\n");
    fd = open("/dev/cryptomodule", O_RDWR); // Abrir o arquivo com permissao para escrita e leitura
    if (fd < 0)
    {
        perror("Erro ao iniciar o modulo...");
        return errno;
    }
    printf("Digite a string que sera enviada:\n");
    scanf("%[^\n]%*c", stringToSend); // String que sera envida para o modulo
    printf("Escrevendo a mensagem no modulo [%s].\n", stringToSend);
    ret = write(fd, stringToSend, strlen(stringToSend)); // envio da string para o modulo
    if (ret < 0)
    {
        perror("Erro no envio da string.");
        return errno;
    }

    printf("Aberte ENTER para ler a mensagem..\n");
    getchar();

    printf("Lendo do modulo...\n");
    ret = read(fd, receive, BUFFER_LENGTH); // Leitura do buffer
    if (ret < 0)
    {
        perror("Erro na leitura da mensagem.");
        return errno;
    }
    printf("Mensagem recebida: [%s]\n", receive);
    printf("Final do programa\n");
    return 0;
}