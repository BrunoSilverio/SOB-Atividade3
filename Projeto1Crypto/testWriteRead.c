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
    int ret, fd, opcao;
    char stringToSend[BUFFER_LENGTH];

    while (opcao != 3)
    {
        printf("Opcao 1: Escrever no modulo\n");
        printf("Opcao 2: Ler do modulo\n");
        printf("Opcao 3: sair\n");

        scanf("%i", opcao);

        switch (opcao)
        {
        case 1:
            printf("Iniciando o modulo...\n");
            fd = open("/dev/cryptomodule", O_RDWR); // Abrir o arquivo com permissao para escrita e leitura
            if (fd < 0)
            {
                perror("Erro ao iniciar o modulo...");
                return errno;
            }
            printf("Digite a string que sera enviada:\n");
            scanf("%[^\n]%*c", stringToSend); // String que sera envida para o modulo
            printf("Escrevendo a mensagem [%s] no modulo .\n", stringToSend);
            ret = write(fd, stringToSend, strlen(stringToSend)); // envio da string para o modulo
            if (ret < 0)
            {
                perror("Erro no envio da string.");
                return errno;
            }
            printf("Aberte ENTER para voltar ao menu..\n");
            getchar();
            break;

        case 2:
            printf("Lendo do modulo...\n");
            ret = read(fd, receive, BUFFER_LENGTH); // Leitura do buffer
            if (ret < 0)
            {
                perror("Erro na leitura da mensagem.");
                return errno;
            }
            printf("Mensagem recebida: [%s]\n", receive);
            printf("Aberte ENTER para voltar ao menu..\n");
            getchar();
            break;
        case 3:
            printf("Final do programa\n");
            break;
        }
    }

    
    return 0;
}
