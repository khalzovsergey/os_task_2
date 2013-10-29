// printf
#include <stdio.h>
// malloc
#include <stdlib.h>


#include <sys/types.h>
#include <sys/socket.h>

// struct sockaddr_un
//#include <sys/un.h>

#include <signal.h>
#include <unistd.h>


// IPPORT_RESERVED
//#include <netdb.h>

// struct sockaddr_in
#include <netinet/in.h>


#define _DEBUG

#ifdef _DEBUG

#define TRACE printf("%s; %d\n", __FILE__, __LINE__);
//#define TRACE printf("main.c; %d\n", __LINE__);

#else

#define TRACE

#endif // _DEBUG


#define SIZE_OF_QUEUE 10
#define INPORT 9998
#define TRUE 1
#define FALSE 0


int process_request(int client_sockfd);


int main(int avgc, char *argv[])
{
    // объвления переменных для сервера
    int server_sockfd;
    struct sockaddr_in server_address;

    // создать слушающий сокет
    server_sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (server_sockfd == -1)
    {
        // напечатать сообщение об ошибке создания сокета
        perror("server socket error");
        exit(EXIT_FAILURE);
    }

    // именовать сокет
    server_address.sin_family = AF_INET;
    server_address.sin_addr.s_addr = htonl(INADDR_ANY);
    server_address.sin_port = htons(INPORT);
    if (bind(server_sockfd, (struct sockaddr *)&server_address, (socklen_t)sizeof(server_address)))
    {
        // напечатать сообщение об ошибке именования сокета
        perror("server bind error");
        close(server_sockfd);
        exit(EXIT_FAILURE);
    }

    // создать очередь соединений
    if (listen(server_sockfd, SIZE_OF_QUEUE))
    {
        // напечатать сообщение об ошибке создания очереди соединений
        perror("server listen error");
        close(server_sockfd);
        exit(EXIT_FAILURE);
    }

    // игнорировать статус завершения дочернего процесса для устранения появления зомби-процессов
    signal(SIGCHLD, SIG_IGN);

    // счетчик ветвлений
    int fork_count = 0;
    // счетчик неудачных ветвлений
    int fork_error_count = 0;

    // объвления переменных для клиента
    int client_sockfd;
    socklen_t client_len;
    struct sockaddr_in client_address;

    // цикл обработки запросов
    while (TRUE)
    {
        client_len = sizeof(client_address);
        client_sockfd = accept(server_sockfd, (struct sockaddr *)&client_address, &client_len);
        fork_count++;
        switch (fork())
        {
            case -1:
                // напечатать сообщение об ошибке ветвления
                perror("server fork error");
                // увеличить счетчик ошибок ветвления
                fork_error_count++;
                // обработать клиента в головном процессе
                process_request(client_sockfd);
                break;
            case 0:
                // обработать клиента в дочернем процессе процессе
                exit(process_request(client_sockfd));
                break;
            default:
                close(client_sockfd);
                break;
        }
    }

    // закрыть слушающий сокет
    //close(server_sockfd);
    // завершить программу
    //exit(EXIT_SUCCESS);

/*
    // максимальный порт
    //printf("%d\n", IPPORT_RESERVED);

    // неблокирующее ожидание
    //int flags = fcntl(socket, F_GETFL, 0);
    //fnctl(socket, F_SETFL, O_NONBLOCK | flags);

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
*/

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
    //exit(EXIT_SUCCESS);
}


#define BUF_SIZE 4096


int process_request(int client_sockfd)
{
    char *buf[BUF_SIZE];// = (char *)malloc(BUF_SIZE * sizeof(char));
    int result = 0;
    int count = 0;
    while (count <= BUF_SIZE && (result = read(client_sockfd, buf + count, BUF_SIZE - count)))
    {
        if (result == -1)
        {
            close(client_sockfd);
            return EXIT_FAILURE;
        }
        count += result;
    }



    //free(buf);
    close(client_sockfd);
    //return EXIT_FAILURE;
    return EXIT_SUCCESS;
}

















