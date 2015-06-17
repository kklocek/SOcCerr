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
struct playerInfo info[CLIENTS];
int idx = 0;

void cleaner();
void setup();
void work();
void check();
void handleRequest(int i, struct serverCaller caller);
void registerPeer(int i, struct serverCaller caller);
void handleNewConnection(int s);
void findPeer(int sock, struct serverCaller caller, int isSpecific);
void sendResponse(int first, int second, int status);
int findIndex(struct serverCaller caller);
void closePeer(int i);

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
        info[i].sock = -1;
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
        printf("WORKING...\n");
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
                else
                {
                    struct serverCaller caller;
                    int len = recv(i, &caller, sizeof(struct serverCaller), 0); ///TODO: Przemyslec IPC_NOWAIT

                    if(len != 0)
                    {
                        handleRequest(i, caller);
                    }
                    else
                    {
                        printf("Len = 0 with %d, closing connection...\n", i);
                        closePeer(i);
                        close(1);
                    }

                }
            }
        }

    }
}

void handleRequest(int i, struct serverCaller caller)
{
    if(caller.request == FIND)
        findPeer(i, caller, 0);
    else if(caller.request == FIND_SPECIFIC)
        findPeer(i, caller, 1);
    else if(caller.request == REGISTER)
        registerPeer(i, caller);
}

void handleNewConnection(int s)
{


}

int findIndex(struct serverCaller caller)
{
    int j;
    for(j = 0; j < idx; j++)
    {
        if(strcmp(caller.id, info[j].id) == 0)
        {
            return j;
        }
    }

    return 0;
}

void findPeer(int sock, struct serverCaller caller, int isSpecific)
{
    printf("Finding peer...\n");
    int j;
    int myIdx = findIndex(caller);
    for(j = 0; j < idx; j++)
    {
        //printf("j = %d\nStatus: %d", j, info[j].status);
        if(info[j].status == WAITING && (strcmp(info[j].id, info[myIdx].id) != 0))
        {
            //printf("In if: info[j] = %s\n", info[j].id);
            //printf("Specific: %d\ncaller.id: %s\n", isSpecific, info[myIdx].id);
            if(!isSpecific || (isSpecific && strcmp(info[j].id, caller.id2) == 0))
            {
                //printf("In if2.\n");
                info[j].status = PLAYING;
                info[myIdx].status = PLAYING;
                //printf("In if22.\n");
                //No to jazda
                strcpy(caller.id2, info[j].id);
                caller.msg.idOneTurn = 1;
                caller.msg.x = PITCH_X_SIZE / 2;
                caller.msg.y = PITCH_Y_SIZE / 2;
                caller.response = ACCEPTED;
                //sendResponse(caller.peerSocket, info[j].status, ACCEPTED);
                //printf("FOUND!\n");
                if(send(sock, &caller, sizeof(struct clientCaller), 0) == -1)
                {
                    perror("SEND RESPONSE FIND PEER ERROR");
                    exit(1);
                }

                if(send(info[j].sock, &caller, sizeof(struct clientCaller), 0) == -1)
                {
                    perror("SEND RESPONSE FIND PEER ERROR");
                    exit(1);
                }


                //Teraz do drugiego jeszcze :P
                printf("NEW GAME STARTED: %s vs %s!\n", caller.id, caller.id2);
                return;

            }
        }
    }

    //Nie ma nikogo, smutne :/
    //W takim razie czekamy na kolejnego, lub dajemy komunikat ze nie ma takiego specyficznego
    if(isSpecific)
    {
        printf("%s couldn't find specific player.\n", caller.id2);
        caller.response = NOTACCEPTED;
        if(send(sock, &caller, sizeof(struct clientCaller), 0) == -1)
        {
            perror("SEND RESPONSE FIND PEER ERROR");
            exit(1);
        }

    }
    else
    {
        printf("%s is waiting for other player.\n", caller.id);
        info[myIdx].status = WAITING;
    }

}
void registerPeer(int i, struct serverCaller caller)
{
    //Sprawdzamy czy mamy juz takiego
    int j;
    printf("Connection from %s...\n", caller.id);
    for(j = 0; j < idx; j++)
    {
        ///TODO: Co jak mamy juz takiego klienta?
        if(info[j].sock != -1 && strcmp(info[j].id, caller.id) == 0)
        {
            printf("%s have been registered.\n", caller.id);
            return;
        }
    }

    ///TODO: Zrobic to ladniej
    printf("New client registered: %s\n", caller.id);
    strcpy(info[idx].id, caller.id);
    info[idx].sock = i;
    info[idx].status = BEING;
    idx++;

}

void closePeer(int i)
{
    int j;
    for(j = 0; j < idx; j++)
    {
        if(info[j].sock == i)
        {
            info[j].sock = -1;
            strcpy(info[j].id, "");
            break;
        }
    }

}

void cleaner()
{
    if(close(mySocket) == -1)
    {
        perror("CLOSE NET SOCKET ERROR.");
        exit(1);
    }

}
