#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <errno.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <netdb.h>
#include <string.h>
#include <arpa/inet.h>
extern int errno;

int port;

int main(int argc, char *argv[])
{
    int sd, ok = 1;
    struct sockaddr_in server;
    char msg[8192];
    if (argc != 3)
    {
        printf("Sintaxa: %s <adresa_server> <port>\n", argv[0]);
        return -1;
    }
    port = atoi(argv[2]);
    if ((sd = socket(AF_INET, SOCK_STREAM, 0)) == -1)
    {
        perror("Eroare la socket().\n");
        return errno;
    }
    server.sin_family = AF_INET;
    server.sin_addr.s_addr = inet_addr(argv[1]);
    server.sin_port = htons(port);
    if (connect(sd, (struct sockaddr *)&server, sizeof(struct sockaddr)) == -1)
    {
        perror("Eroare la connect().\n");
        return errno;
    }

    while (ok)
    {
        bzero(msg, 8192);
        printf("\e[1;32m[server]Introduceti o comanda(quit|comenzi): \e[0m");
        fflush(stdout);
        read(0, msg, 8192);
        // printf("[%s]\n",msg);
        if (!strcmp(msg, "quit\n") || !strcmp(msg, "Quit\n") || !strcmp(msg, "quit") || !strcmp(msg, "Quit") || !strcmp(msg, "13.\n") || !strcmp(msg, "13."))
        {
            ok = 0;
            if (write(sd, msg, 8192) <= 0)
            {
                perror("[client]Eroare la write() spre server.\n");
                return errno;
            }
        }
        else
        {
            if (write(sd, msg, 8192) <= 0)
            {
                perror("Eroare la write() spre server.\n");
                return errno;
            }

            if (read(sd, msg, 8192) < 0)
            {
                perror("Eroare la read() de la server.\n");
                return errno;
            }
            printf("Mesajul primit este:\n%s\n", msg);
            if (ok == 0)
                close(sd);
        }
    }
    close(sd);
}
