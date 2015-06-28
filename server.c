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
char portS[50];
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
void findPeer(int sock, struct serverCaller caller, int isSpecific);
void sendResponseToClient(int sock, struct serverCaller caller, int status);
void sendResponse(int first, int second, int status);
int findIndex(struct serverCaller caller);
void closePeer(int i);
int stillPlaying(int i);
void nextTurn(int i, struct serverCaller caller);
void getScore(int i, struct serverCaller caller);

int main(int argc, char** argv)
{
    if(argc != 2)
    {
        fprintf(stderr, "USAGE: <port>\n");
        exit(0);
    }

    strcpy(portS, argv[1]);
    int i;
    for(i = 0; i < CLIENTS; i++)
    {
        info[i].sock = -1;
    }

    struct sigaction s;
    sigset_t set;
    sigemptyset(&set);
    s.sa_flags = SA_RESTART;
    s.sa_mask = set;
    s.sa_handler = handlerS;
    if(sigaction(SIGINT, &s, NULL) == -1)
    {
        perror("SIGACTION ERROR");
        exit(1);
    }
    atexit(cleaner);
    setup();
    work();
    return 0;
}

/**
	setup - ustanowienie polaczenia
*/
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
    my_addr.sin_port = htons(atoi(portS)); //network byt order
    my_addr.sin_addr.s_addr = htonl(INADDR_ANY); //Moje ip
    //Niezbedne zerowanie
    memset(&(my_addr.sin_zero), '\0', 8);

    if(bind(mySocket, (struct sockaddr*)&my_addr, sizeof(struct sockaddr)) == -1)
    {
        perror("BIND ERROR");
        exit(1);
    }

}

/**
	work - petla dzialania
*/
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

                        printf("New connection by net: %d.\n", newfd);
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
                        FD_CLR(i, &mainer);
                        //close(i);
                    }

                }
            }
        }

    }
}

/**
	handleRequest - oddelegowanie zadania do specyficznej funkcji
*/
void handleRequest(int i, struct serverCaller caller)
{
    if(caller.request == FIND)
        findPeer(i, caller, 0);
    else if(caller.request == FIND_SPECIFIC)
        findPeer(i, caller, 1);
    else if(caller.request == REGISTER)
        registerPeer(i, caller);
    else if(caller.request == TURN)
        nextTurn(i, caller);
    else if(caller.request == GET_RECORD)
        getScore(i, caller);
}

/**
	findIndex - Wyszukiwanie elementu ze struktury w tablicy graczy
*/
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

/**
	findPeer - wyszukiwanie gracza do gry
*/
void findPeer(int sock, struct serverCaller caller, int isSpecific)
{
    printf("Finding peer...\n");
    int j;
    int myIdx = findIndex(caller);
    for(j = 0; j < idx; j++)
    {
        if(info[j].sock != -1 && info[j].status == WAITING && (strcmp(info[j].id, info[myIdx].id) != 0))
        {
            if(!isSpecific || (isSpecific && strcmp(info[j].id, caller.id2) == 0))
            {
                info[j].status = PLAYING;
                info[myIdx].status = PLAYING;

                strcpy(caller.id2, info[j].id);
                caller.msg.idOneTurn = 1;
                caller.msg.x = PITCH_X_SIZE / 2;
                caller.msg.y = PITCH_Y_SIZE / 2 + 1;
                caller.msg.sock1 = sock;
                caller.msg.sock2 = info[j].sock;
                caller.response = ACCEPTED;
                caller.msg.isEnded = 0;

                if(send(sock, &caller, sizeof(struct serverCaller), 0) == -1)
                {
                    perror("SEND RESPONSE FIND PEER ERROR");
                    exit(1);
                }

                if(send(info[j].sock, &caller, sizeof(struct serverCaller), 0) == -1)
                {
                    perror("SEND RESPONSE FIND PEER ERROR");
                    exit(1);
                }


                printf("NEW GAME STARTED: %s vs %s!\n", caller.id, caller.id2);
                return;

            }
        }
    }

    //Nie ma nikogo
    //W takim razie czekamy na kolejnego, lub dajemy komunikat ze nie ma takiego specyficznego
    if(isSpecific)
    {
        printf("%s couldn't find specific player.\n", caller.id2);
        caller.response = NOTACCEPTED;
        printf("Sending notaccepted: %d\n", caller.response);
        if(send(sock, &caller, sizeof(struct serverCaller), 0) == -1)
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

/**
	registerPeer - rejestracja gracza
*/
void registerPeer(int i, struct serverCaller caller)
{
    //Sprawdzamy czy mamy juz takiego
    int j;
    printf("Connection from %s...\n", caller.id);
    for(j = 0; j < idx; j++)
    {
        if(info[j].sock != -1 && strcmp(info[j].id, caller.id) == 0)
        {
            printf("%s have been registered.\n", caller.id);
            sendResponseToClient(i, caller, NOTACCEPTED);
            return;
        }
    }

    printf("New client registered: %s\n", caller.id);
    strcpy(info[idx].id, caller.id);
    info[idx].sock = i;
    info[idx].status = BEING;
    idx++;
    sendResponseToClient(i, caller, ACCEPTED);
}

/**
	nextTurn - obsluga zadania nastepnej tury rozgrywki
*/
void nextTurn(int i, struct serverCaller caller)
{
    printf("Next turn. Id1 = %s, Id2 = %s\n", caller.id, caller.id2);
    int iAmFirst = (i == caller.msg.sock1) ? 1 : 0;
    //Co jesli nasz klient sie odlaczyl?

    if(iAmFirst)
    {
        if(!stillPlaying(i))
        {
            caller.msg.isEnded = 1;
            caller.msg.status = PLAYER_TWO_SURRENDING;
            if(send(caller.msg.sock1, &caller, sizeof(struct serverCaller), 0) == -1)
            {
                perror("SEND ERROR NEXT TURN");
                exit(1);
            }
        }
        else
        {
            if(send(caller.msg.sock2, &caller, sizeof(struct serverCaller), 0) == -1)
            {
                perror("SEND ERROR NEXT TURN");
                exit(1);
            }
        }
    }
    else
    {
        if(!stillPlaying(i))
        {
            caller.msg.isEnded = 1;
            caller.msg.status = PLAYER_ONE_SURRENDING;
            if(send(caller.msg.sock2, &caller, sizeof(struct serverCaller), 0) == -1)
            {
                perror("SEND ERROR NEXT TURN");
                exit(1);
            }
        }
        else
        {
            if(send(caller.msg.sock1, &caller, sizeof(struct serverCaller), 0) == -1)
            {
                perror("SEND ERROR NEXT TURN");
                exit(1);
            }
        }

    }

    FILE* fp;
    fp = fopen(strcat(caller.id, caller.id2), "a");
    if(fp == NULL)
    {
        perror("FOPEN ERROR");
        exit(1);
    }

    fprintf(fp, "%d %d\n", caller.msg.x, caller.msg.y);
    if(fclose(fp) == -1)
    {
        perror("FCLOSE ERROR");
        exit(1);
    }

    if(caller.msg.isEnded)
    {
        //Koniec gry, mozemy jeszcze raz zagrac
        closeGame(caller.msg.sock1);
        closeGame(caller.msg.sock2);
    }
}

/**
	closePeer - usuniecie gracza z gry
*/
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

/**
	sendResponseToClient - wyslanie do klienta odpowiedniej wiadomosci
*/
void sendResponseToClient(int sock, struct serverCaller caller, int status)
{
    caller.response = status;
    if(send(sock, &caller, sizeof(struct serverCaller), 0) == -1)
    {
        perror("SEND RESPONSE TO CLIENT ERROR.");
        exit(1);
    }
}

/**
	handlerS - obsluga sygnalu SIGINT
*/
void handlerS(int sig)
{
    exit(0);
}

/**
	cleaner - funkcja czyszczaca nasze polaczenia
*/
void cleaner()
{
    //Zamykamy wszystkie polaczenia
	/*
    int j;
    for(j = 0; j < idx; j++)
    {
        close(info[j].sock);
    }
	*/

    if(close(mySocket) == -1)
    {
        perror("CLOSE NET SOCKET ERROR.");
        exit(1);
    }

}

/**
	closeGame - ustanawia status danego gracza na nie rozgrywajacego meczu
*/
void closeGame(int i)
{
    int j;
    for(j = 0; j < idx; j++)
    {
        if(info[j].sock == i)
        {
            info[j].status = BEING;
            break;
        }
    }

}

/**
	stillPlaying - sprawdzenie czy dany gracz gra
*/
int stillPlaying(int i)
{
    int j;
    for(j = 0; j < idx; j++)
    {
        if(info[j].sock == i && info[j].status != PLAYING)
            return 0;
    }
    return 1;
}

/**
	getScore - zwrocenie zapisu meczu
*/
void getScore(int i, struct serverCaller caller)
{
    FILE* fp;
    int fail = 0;
    char buf2[30];
    strcpy(buf2, caller.id);
    fp = fopen(strcat(buf2, caller.id2), "r");
    if(fp == NULL)
    {
        fp = fopen(strcat(caller.id2, caller.id), "r");
        if(fp == NULL)
            fail = 1;
    }

    if(fail)
    {
        if(send(i, "", 1, 0) == -1)
        {
            perror("SEND ERROR");
            exit(1);
        }
        return;
    }

    char buf[320 * 6];
    fread(buf, 1, 320 * 6, fp);
    printf("%s\n", buf);
    if(send(i, buf, 320 * 6, 0) == -1)
    {
        perror("SEND ERROR");
        exit(1);
    }

    if(fclose(fp) == -1)
    {
        perror("FCLOSE ERROR");
        exit(1);
    }
}
