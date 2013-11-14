#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <signal.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <fcntl.h>


#define _DEBUG

#ifdef _DEBUG

#define TRACE printf("%s; %d\n", __FILE__, __LINE__);

#else

#define TRACE

#endif // _DEBUG


#define SIZE_OF_QUEUE 10
#define INPORT 1025
#define TRUE 1
#define FALSE 0
#define INADDR "127.0.0.1"
// размер буфера
#define BUF_SIZE 4096


// обработать запрос
int process_query(int client_sockfd);
// закрыть сервер
void close_server(int sig);


int main(int avgc, char *argv[])
{
    // закрытие сервера
    struct sigaction act;
    act.sa_handler = close_server;
    sigemptyset(&act.sa_mask);
    act.sa_flags = 0;
    sigaction(SIGINT, &act, 0);

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
    server_address.sin_addr.s_addr = inet_addr(INADDR);
    server_address.sin_port = htons(INPORT);
    if (bind(server_sockfd, (struct sockaddr *)&server_address, (socklen_t)sizeof(server_address)))
    {
        // напечатать сообщение об ошибке именования сокета
        perror("server bind error");
        //close(server_sockfd);
        exit(EXIT_FAILURE);
    }

    // создать очередь соединений
    if (listen(server_sockfd, SIZE_OF_QUEUE))
    {
        // напечатать сообщение об ошибке создания очереди соединений
        perror("server listen error");
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
                // обработать клиента в дочернем процессе процессе
                exit(process_query(client_sockfd));
                break;
            default:
                // закрыть клиентский сокет
                close(client_sockfd);
                break;
        }
    }

    //exit(EXIT_FAILURE);
    //exit(EXIT_SUCCESS);
}


// копировать данные из одного файлового дескриптора в другой файловый дескриптор
int copy(int fd_sour, int fd_dest)
{
    void *buf[BUF_SIZE];
    int count;
    int result = 0;
    while (TRUE)
    {
        count = read(fd_sour, buf, BUF_SIZE);
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


#define METHOD_GET "GET"
#define METHOD_NOT_IMPLEMENTED "HTTP/1.1 501 Not Implemented\r\n\r\n<html><body><h1>501 Not Implemented</h1></body></html>"
#define ROOT "/home/sergey"
#define INDEX "/home/sergey/index.html"
#define OK "HTTP/1.1 200 OK\r\n\r\n"
#define NOT_FOUND "HTTP/1.1 404 Not Found\r\n\r\n<html><body><h1>404 Not Found</h1></body></html>"


int process_query(int client_sockfd)
{
    char buf[BUF_SIZE];

    int count = read(client_sockfd, buf, BUF_SIZE - 1);
    if (count <= 0)
    {
        return EXIT_FAILURE;
    }

    buf[count] = '\0';

    char *method = strtok(buf, " ");
    char *uri = strtok(NULL, " ");

    if (strcmp(method, METHOD_GET) == 0)
    {
        int fd;
        if (strcmp(uri, "/") == 0)
        {
            fd = open(INDEX, O_RDONLY);
        }
        else
        {
            char path[BUF_SIZE];
            strcpy(path, ROOT);
            strcpy(path + sizeof(ROOT) - 1, uri);
            fd = open(path, O_RDONLY);
        }
        if (fd >= 0)
        {
            write(client_sockfd, OK, sizeof(OK) - 1);
            copy(fd, client_sockfd);
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


void close_server(int sig)
{
    printf("\nserver is closed\n");
    exit(EXIT_SUCCESS);
}
