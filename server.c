//Serwer zajmuje się przyjmowaniem połąceń - w razie nowego połączenia tworzy nowy wątek do obsługi graczy
#define CLIENTS 100
#define BACKLOG 10
#define TIMEOUT 30

#include <stdio.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/select.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/un.h>
#include <time.h>

#include "header.h"
char port[50];
char path[50];
int mySocket;
struct peerInfo info[CLIENTS];
int idx = 0;

void cleaner();
void setup();
void work();
void check();
void handleRequest(int i, struct message msg, fd_set set, int max);
void registerPeer(int i, struct message msg, struct sockaddr remoteAddr);
void handleNewConnection(int s);

int main(int argc, char** argv)
{
    if(argc != 2)
    {
        fprintf(stderr, "USAGE: <port>\n");
        exit(0);
    }

    strcpy(port, argv[1]);
    int i;
    for(i = 0; i < CLIENTS; i++)
    {
        info[i].peerSocket = -1;
    }
    atexit(cleaner);
    setup();
    work();
    return 0;
}

void setup()
{
    mySocket = socket(PF_INET, SOCK_STREAM, IPPROTO_TCP);
    if(mySocket == -1)
    {
        perror("SOCKET ERROR");
        exit(1);
    }
    struct sockaddr_in my_addr;

    my_addr.sin_family = AF_INET; //Host byte order
    my_addr.sin_port = htons(atoi(port)); //network byt order
    my_addr.sin_addr.s_addr = htonl(INADDR_ANY); //Moje ip
    //Niezbedne zerowanie
    memset(&(my_addr.sin_zero), '\0', 8);

    if(bind(mySocket, (struct sockaddr*)&my_addr, sizeof(struct sockaddr)) == -1)
    {
        perror("BIND ERROR");
        exit(1);
    }

}

void work()
{
    fd_set mainer, reader;
    int max;
    struct sockaddr remoteAddr;

    unsigned int l = sizeof(remoteAddr);
    struct message msg;
    FD_ZERO(&mainer);
    FD_ZERO(&reader);
    FD_SET(mySocket,&mainer);
    max = mySocket;

    if(listen(mySocket, CLIENTS) == -1)
    {
        perror("LISTEN NET ERROR");
        exit(1);
    }



    while(1)
    {
        reader = mainer;
        if(select(max + 1, &reader, NULL, NULL, NULL) == -1)
        {
            perror("SELECT ERROR");
            exit(1);
        }

        int i;
        for(i = 0; i <= max; i++)
        {
            if(FD_ISSET(i, &reader))
            {
                if(i == mySocket)
                {
                    int newfd;
                    if((newfd = accept(mySocket, (struct sockaddr*)&remoteAddr, &l)) == -1)
                    {
                        perror("ACCEPT ERROR");
                        exit(1);
                    }
                    else
                    {
                        FD_SET(newfd, &mainer);
                        if(newfd > max)
                            max = newfd;

                        printf("New connection by net: %d.\n", i);
                        handleNewConnection(newfd);
                    }
                }
            }
        }
        printf("WORKING...\n");
    }
}

void handleRequest(int i, struct message msg, fd_set set, int max)
{

    //printf("%s: %s\n", msg.name, msg.msg);
    int j;

    for(j = 0; j <= max; j++)
    {
        if(FD_ISSET(j, &set))
        {
            if(j != mySocket)
            {
                if(send(j, &msg, sizeof(struct message), 0) == -1)
                {
                    perror("SEND ERROR");
                    exit(1);
                }
            }
        }

    }
}

void handleNewConnection(int s)
{


}

void cleaner()
{
    if(close(mySocket) == -1)
    {
        perror("CLOSE NET SOCKET ERROR.");
        exit(1);
    }

}
