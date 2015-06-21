#include "header.h"

//extern int currX, currY, iAmFirst, mySocket;
//extern char ip[30];
/**
    estabilishNetConnection - nawiazanie polaczenia
*/
void estabilishNetConnection()
{
    ///TODO: Obsluga zlych parametrow
    printf("Connecting to %s:%d...\n", ip, port);
    remoteAddr.sin_family = AF_INET;
    remoteAddr.sin_port = htons(port);
    remoteAddr.sin_addr.s_addr = inet_addr(ip);
    memset(&(remoteAddr.sin_zero), '\0', 8);

    mySocket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if(mySocket == -1)
    {
        perror("SOCKET ERROR");
        exit(1);
    }

    if(connect(mySocket, (struct sockaddr*)&remoteAddr, sizeof(struct sockaddr_in)) == -1)
    {
        perror("CONNECT NET ERROR");
        exit(1);
    }
    printf("CONNECTED!\n");


}


/**
    findPlayer - szukamy gracza
*/
void findPlayer(int type)
{
    struct serverCaller call;
    //struct clientCaller call2;
    strcpy(call.id, id);
    call.request = FIND;
    if(type)
    {
        char id2[30];
        printf("Please input second player id: ");
        scanf("%s", id2);
        strcpy(call.id2, id2);
        call.request = FIND_SPECIFIC;
    }


    ///TODO: Timed response!

    printf("Sending request...\n");
    if(send(mySocket, &call, sizeof(struct serverCaller),0) == -1)
    {
        perror("SEND ERROR");
        exit(1);
    }

    if(recv(mySocket, &call, sizeof(struct serverCaller), 0) == -1)
    {
        perror("RECV ERROR");
        exit(1);
    }

    if(call.response == ACCEPTED)
    {
        //jest dobrze, mamy z kim grać
        //GRAMY!
        //printf("Id1 = %s, id2 = %s, myId = %s.\n", call.id, call.id2, id);
        if(strcmp(id, call.id) == 0)
            iAmFirst = 1;
        else
            iAmFirst = 0;
        //printf("iAmFirst: %d\n", iAmFirst);

        if(iAmFirst)
            printf("Your rival: %s. You are first. Starting game...\n", call.id2);
        else
            printf("Your rival: %s. You are second. Starting game...\n", call.id);

        playGame(call); //W call2 mamy zapisane id nasze i 'tego drugiego' :P
        ///TODO: Zrobić coś jeżeli nie chcę grać z tym gościem?
    }
    else if(type)
    {
        //Nie udalo nam sie znalezc tego jedynego...
        printf("Sorry, but %s is not connected. What do you want to do?\n", call.id2);
        printf("1. Find new player\n2. Find new specific player\n3. Back to main menu.\n");
        int c;
        scanf("%d", &c);

        switch(c)
        {
            case 1:
                findPlayer(0);
                break;
            case 2:
                findPlayer(1);
                break;
            case 3:
                menu();
                break;
            default:
                printf("Wrong option, back to menu!\n"); ///TODO: Zrobic to lepiej?
                menu();
        }
    }
    else
    {
        printf("There are no peers which could play with you... We will try again in the moment!\n");
        sleep(1);

    }

}


/**
    receive - Tutaj odbieramy wiadomosci od servera
*/
struct serverCaller receive()
{
    struct serverCaller call;
    printf("Waiting for response...\n");
    if(recv(mySocket, &call, sizeof(struct serverCaller), 0) == -1)
    {
        perror("RECV IN GAME ERROR");
        exit(1);
    }
    actualize(call);
    drawPitch();
    return call;
}

/**
    sender - wyslanie wiadomosci
*/
void sender(struct serverCaller call)
{
    if(send(mySocket, &call, sizeof(struct serverCaller), 0 ) == -1)
    {
        perror("SEND ERROR IN SENDER");
        exit(1);
    }
}

/**
    registerName - rejestracja na serwerze
*/
void registerName()
{
    struct serverCaller call;
    strcpy(call.id, id);
    call.request = REGISTER;

    if(send(mySocket, &call, sizeof(struct serverCaller), 0) == -1)
    {
        perror("SEND REGISTER ERROR.");
        exit(1);
    }

}
