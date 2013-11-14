#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <signal.h>
#include <unistd.h>
#include <netinet/in.h>
#include <fcntl.h>


#define TRUE 1
#define FALSE 0
#define INPORT 1025
#define SIZE_OF_QUEUE 10


// обработать запрос
static int process_query(int client_sockfd);
// закрыть сервер
static void close_server(int sig);


int main(int avgc, char *argv[])
{
    // закрытие сервера при получении сигнала SIGINT
    {
        struct sigaction act;
        act.sa_handler = close_server;
        sigemptyset(&act.sa_mask);
        act.sa_flags = 0;
        sigaction(SIGINT, &act, 0);
    }

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
    server_address.sin_addr.s_addr = htonl(INADDR_ANY);//inet_addr("127.0.0.1");
    server_address.sin_port = htons(INPORT);
    if (bind(server_sockfd, (struct sockaddr *)&server_address, (socklen_t)sizeof(server_address)))
    {
        // напечатать сообщение об ошибке именования сокета
        perror("server bind error");
        // закрыть слушающий сокет
        //close(server_sockfd);
        exit(EXIT_FAILURE);
    }

    // создать очередь соединений
    if (listen(server_sockfd, SIZE_OF_QUEUE))
    {
        // напечатать сообщение об ошибке создания очереди соединений
        perror("server listen error");
        // закрыть слушающий сокет
        //close(server_sockfd);
        exit(EXIT_FAILURE);
    }

    // игнорировать статус завершения дочернего процесса для устранения появления зомби-процессов
    signal(SIGCHLD, SIG_IGN);

    // объвления переменных для клиента
    int client_sockfd;
    socklen_t client_len;
    struct sockaddr_in client_address;

    // номер запроса
    int query_number = 0;

    // цикл обработки запросов
    printf("server waiting...\n");
    while (TRUE)
    {
        client_len = sizeof(client_address);
        client_sockfd = accept(server_sockfd, (struct sockaddr *)&client_address, &client_len);
        printf("query number = %d\n", query_number++);
        switch (fork())
        {
            case -1:
                // напечатать сообщение об ошибке ветвления
                perror("server fork error");
                // закрыть клиентский сокет
                close(client_sockfd);
                break;
            case 0:
                // закрыть слушающий сокет
                close(server_sockfd);
                // обработать клиента в дочернем процессе процессе
                exit(process_query(client_sockfd));
                //int result = process_query(client_sockfd);
                // закрыть клиентский сокет
                //close(client_sockfd);
                // закрыть процесс
                //exit(result);
                break;
            default:
                // закрыть клиентский сокет
                close(client_sockfd);
                break;
        }
    }
}


#define COPY_BUF_SIZE 4096


// копировать данные из одного файлового дескриптора в другой файловый дескриптор
static int copy(int fd_sour, int fd_dest)
{
    char buf[COPY_BUF_SIZE];
    int count;
    int result = 0;
    while (TRUE)
    {
        count = read(fd_sour, buf, sizeof(buf));
        if (count <= 0)
        {
            break;
        }
        count = write(fd_dest, buf, count);
        if (count <= 0)
        {
            break;
        }
        result += count;
    }
    return result;
}


#define QUAERY_BUF_SIZE 4096
#define PATH_BUF_SIZE 4096
#define METHOD_GET "GET"
#define METHOD_NOT_IMPLEMENTED "HTTP/1.1 501 Not Implemented\r\n\r\n<html><body><h1 align=\"center\">501 Not Implemented</h1></body></html>"
#define ROOT "/"
#define HOME "/home/sergey"
#define INDEX "/home/sergey/index.html"
#define OK "HTTP/1.1 200 OK\r\n\r\n"
#define NOT_FOUND "HTTP/1.1 404 Not Found\r\n\r\n<html><body><h1 align=\"center\">404 Not Found</h1></body></html>"


// обработать запрос
static int process_query(int client_sockfd)
{
    char buf[QUAERY_BUF_SIZE];

    int count = read(client_sockfd, buf, sizeof(buf) - 1);
    if (count <= 0)
    {
        return EXIT_FAILURE;
    }

    // защита от выхода за границы буфера
    buf[count] = '\0';

    char *method = strtok(buf, " ");
    char *uri = strtok(NULL, " ");

    if (strcmp(method, METHOD_GET) == 0)
    {
        int fd;
        if (strcmp(uri, ROOT) == 0)
        {
            fd = open(INDEX, O_RDONLY);
        }
        else
        {
            char path[PATH_BUF_SIZE];
            strcpy(path, HOME);
            strcpy(path + sizeof(HOME) - 1, uri);
            fd = open(path, O_RDONLY);
        }
        if (fd >= 0)
        {
            write(client_sockfd, OK, sizeof(OK) - 1);
            copy(fd, client_sockfd);
            close(fd);
        }
        else
        {
            write(client_sockfd, NOT_FOUND, sizeof(NOT_FOUND) - 1);
        }
    }
    else
    {
        write(client_sockfd, METHOD_NOT_IMPLEMENTED, sizeof(METHOD_NOT_IMPLEMENTED) - 1);
    }

    return EXIT_SUCCESS;
}


// закрыть сервер
static void close_server(int sig)
{
    printf("\nserver is closed\n");
    exit(EXIT_SUCCESS);
}
