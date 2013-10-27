#include <stdio.h>
#include <stdlib.h>

#include <sys/types.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <unistd.h>


#define _DEBUG

#ifdef _DEBUG

//#define TRACE printf("%s; %d\n", __FILE__, __LINE__);
#define TRACE printf("main.c; %d\n", __LINE__);

#else

#define TRACE

#endif // _DEBUG


#define SIZE_OF_QUEUE 10


int main(int avgc, char *argv[])
{
    int server_sockfd, client_sockfd;
    socklen_t server_len, client_len;
    struct sockaddr_un server_address;
    struct sockaddr_un client_address;

    // удалить старый сокет
    unlink("server_socket");

    // создать слушающий сокет
    server_sockfd = socket(AF_UNIX, SOCK_STREAM, 0);

    // присвоить имя сокету
    server_address.sun_family = AF_UNIX;
    strcpy(server_address.sun_path, "server_socket");
    server_len = sizeof(server_address);
    bind(server_sockfd, (struct sockaddr *)&server_address, server_len);

    // создать очередь запросов
    listen(server_sockfd, SIZE_OF_QUEUE);

    // работа с клиентами
    int count = 10;
    while (count--)
    {
        char ch;
        printf("server waiting...\n");
        client_len = sizeof(client_address);
        client_sockfd = accept(server_sockfd, (struct sockaddr *)&client_address, &client_len);
        read(client_sockfd, &ch, sizeof(char));
        ch++;
        write(client_sockfd, &ch, sizeof(char));
        close(client_sockfd);
    }

/*
    int sockfd;
    int len;
    struct sockaddr_un address;
    int result;
    char ch = 'A';

    // создать сокет
    sockfd = socket(AF_UNIX, SOCK_STREAM, 0);

    // присвоить имя сокету
    address.sun_family = AF_UNIX;
    strcpy(address.sun_path, "server_socket");
    len = sizeof(address);

    // подключиться к серверу
    result = connect(sockfd, (struct sockaddr *)&address, len);
    if (result == -1)
    {
        perror("connection error");
        exit(EXIT_FAILURE);
    }

    // обмен данными с сокетом
    // послать данные на сервер
    write(sockfd, &ch, sizeof(char));
    // получить данные от сервера
    read(sockfd, &ch, sizeof(char));
    printf("char from server = %c\n", ch);

    // закрыть сокет
    close(sockfd);



    TRACE
*/

    //printf("Hello world!\n");
    //exit(EXIT_FAILURE);
    exit(EXIT_SUCCESS);
}
