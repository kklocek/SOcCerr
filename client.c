#define MSG_LEN 180
#define TIMEOUT 30
#define DTIME 5
#define BUF_SIZE 180

#include <stdio.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/select.h>
#include <pthread.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/un.h>
#include <signal.h>
#include <errno.h>

#include "header.h"
char id[30];
char ip[30];
int port;
///TODO: POMYSLEC NAD TRYBEM VIEWERA
pthread_t ioThread;
pthread_t parent;
pid_t parentPid;
int mySocket = -1; //-1 by sprawdzic czy sie polaczylismy gdzies po prostu
int iAmFirst = 0; //Czy jestem graczem nr 1
struct sockaddr_in remoteAddr;
struct sockaddr_in myAddr;
struct pitch myPitch;

void estabilishNetConnection();
void cleaner();

void menu();
void drawLogo();
void printOptions();
void prepareToGame();
void getHighScore();
void manual();
void findPlayer(int type); //1 - specific, 0 - normal
void playGame(struct serverCaller);
void registerName();
void drawPitch();
void drawNeighbours(struct point point);
struct serverCaller move(struct serverCaller caller);
struct serverCaller receive();
void actualize(struct serverCaller call);
void initPitch();
void sender(struct serverCaller call);

/**
    main - main jak main.
*/
int main(int argc, char** argv)
{
    ///TODO: Mozna dodac parsowanie argumentow wejscia zeby zagrac od razu!
    menu();
    return 0;
}

/**
    menu - menu sobie rysuje
*/
void menu()
{
    drawLogo();
    int choice = 0;
    printf("WELCOME TO SOcCerr - my little project for operating systems!\n");
    do
    {
        printOptions();
        scanf("%d", &choice);
        switch(choice)
        {
            case 1:
                prepareToGame();
                break;
            case 2:
                getHighScore();
                break;
            case 3:
                manual();
                break;
            case 0:
                printf("Thank you, and have a nice day :).\n");
                exit(0);
                break;
            default:
                printf("Wrong option, try again!\n");
                choice = -1;
                break;
        }
    } while(choice != 0);
    printf("Thank you, and have a nice day :)\n");

}

/**
    printOptions - opcje sobie pisze
*/
void printOptions()
{
    printf("\n1. Play a game!\n");
    printf("2. Get a high score table.\n");
    printf("3. Manual :)\n");
    printf("0. Exit.\n");

}

/**
    drawLogo - logo sobie rysuje
*/
void drawLogo()
{
        /*



   _____  ____        _____
  / ____|/ __ \      / ____|
 | (___ | |  | | ___| |     ___ _ __ _ __
  \___ \| |  | |/ __| |    / _ \ '__| '__|
  ____) | |__| | (__| |___|  __/ |  | |
 |_____/ \____/ \___|\_____\___|_|  |_|



    */
    printf("\n");
    printf("   _____  ____        _____\n");
    printf("  / ____|/ __ \\      / ____|\n");
    printf(" | (___ | |  | | ___| |     ___ _ __ _ __\n");
    printf("  \\___ \\| |  | |/ __| |    / _ \\ '__| '__|\n");
    printf("  ____) | |__| | (__| |___|  __/ |  | |\n");
    printf(" |_____/ \\____/ \\___|\\_____\\___|_|  |_|\n\n");

}

/**
    prepareToGame - ustanawia polaczenie i tworzy naszego gracza.
*/
void prepareToGame()
{
    ///TODO: Zapytanie czy z tym samym serwer - może być tak że jestem cały czas połączony, chce sobie poczytać mana, a potem zagrać jeszcze raz
    printf("Input your nickname: ");
    scanf("%s", id);

    printf("Input ip: ");
    scanf("%s", ip);

    printf("Input port: ");
    scanf("%d", &port);


    estabilishNetConnection();
    registerName();
    int answer;
    label:
    printf("Do you want to:\n1. Find a player ? \n2. Play with specific player ? \n0. Back to main menu?\nChoose your option: ");
    scanf("%d", &answer);

    switch(answer)
    {
        case 1:
            findPlayer(0);
            break;
        case 2:
            findPlayer(1);
            break;
        case 0:
            menu();
            break;
        default:
            printf("Wrong option, try again :)\n");
            goto label;
    }
}

/**
    ???
*/
void getHighScore()
{
    //Sprawdzamy czy nie jestesmy polaczeni

}

/**
    manual - wyswietla manual
*/
void manual()
{


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
        printf("Id1 = %s, id2 = %s, myId = %s.\n", call.id, call.id2, id);
        if(strcmp(id, call.id) == 0)
            iAmFirst = 1;
        else
            iAmFirst = 0;
        printf("iAmFirst: %d\n", iAmFirst);

        if(iAmFirst)
            printf("Your rival: %s. Starting game...\n", call.id2);
        else
            printf("Your rival: %s. Starting game...\n", call.id);

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
    playGame - glowna petla gry.
*/
void playGame(struct serverCaller call)
{
    //Cała logika gry
    initPitch();
    do
    {
        while((iAmFirst && !call.msg.idOneTurn) || (!iAmFirst && call.msg.idOneTurn))
            call = receive();
        printf("Received. X = %d, Y = %d.\n", call.msg.x, call.msg.y);
        drawPitch();
        if(call.msg.status != ENDED)
            call = move(call);
        sender(call);
        printf("Sended. X = %d, Y = %d.\n", call.msg.x, call.msg.y);
    } while(call.msg.status != ENDED);
    ///TODO: Ładne zakończenie :P
    printf("\nIt was a good game, thank you :).\n");
}

/**
    ???
*/
void drawNeighbours(struct point point)
{

}

/**
    drawPitch - rysowanie boiska
*/
void drawPitch()
{
    int i, j;
    for(j = 0; j < PITCH_Y_SIZE; j++)
    {
        for(i = 0; i < PITCH_X_SIZE; i++)
        {
            if(myPitch.points[i][j].status == NONE)
                continue;
            else
                drawNeighbours(myPitch.points[i][j]);

        }

    }

}

/**
    move - Tutaj sie ruszamy, w zasadzie logika gry :P
*/
struct serverCaller move(struct serverCaller caller)
{
    //Tu jeszcze jakas petla
    while(1)
    {
        short c = 0;
        while(c > 8 || c < 1)
        {
            printf("Your choice (1 - 8): ");
            scanf("%hd", &c);
            if(c > 8 || c < 1)
                printf("Wrong option, try again!\n");
        }

        int cX = caller.msg.x; //c - current
        int cY = caller.msg.y; //serverCaller przechowuje ostatnie x i y wprowadzone, to juz wystarczy

        //Tablica opisujaca nam jak sie zmienia x i y - indeksem bedzie nasz wybor
        c--; //Dla latwej indeksacji
        int choiceX[] = {-1, -1, 0, 1, 1, 1, 0, -1};
        int choiceY[] = {0, 1, 1, 1, 0, -1, -1, -1};
        int dx = choiceX[c];
        int dy = choiceY[c];

        //Sprawdzamy czy mozemy tam wejsc
        if(myPitch.points[cX][cY].neighbours[c] == 0)
        {
            //Z BUTA WJEZDAM TAM!
            myPitch.points[cX][cY].neighbours[c]= myPitch.points[cX + dx][cY + dy].neighbours[(c + 4) % 8] = 1;

            //Ok, weszlismy na puste pole, mozemy to przeslac dalej
            if(myPitch.points[cX + dx][cY + dy].status == NORMAL)
            {
                myPitch.points[cX + dx][cY + dy].status = TAKEN;
                caller.msg.x = cX;
                caller.msg.y = cY;
                caller.msg.idOneTurn = !caller.msg.idOneTurn;
                caller.request = TURN;
                return caller;
            }

            //TO TERAZ ODBIJANIE, HEHE
            //TRZEBA POSPRAWDZAC TE STATUSY
            //Z UWZGLEDNIENIEM SCIAN
        }
        else
        {
            printf("You can't move there, try again!.\n");
            continue;
        }
    }
}

/**
    receive - Tutaj odbieramy wiadomosci od servera
*/
struct serverCaller receive()
{
    struct serverCaller call;
    if(recv(mySocket, &call, sizeof(struct serverCaller), 0) == -1)
    {
        perror("RECV IN GAME ERROR");
        exit(1);
    }
    actualize(call);
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
    actualize - zmieniamy stan naszego boiska na podstawie wiadomosci
*/
void actualize(struct serverCaller call)
{
    myPitch.points[call.msg.x][call.msg.y].status = TAKEN;
}

/**
    initPitch - Tworzymy nasze boisko.
*/
void initPitch()
{
    //Najpierw X, potem Y
    //Najpierw puste miejsca i linie poziome
    int i, j;
    for(j = 0; j < PITCH_X_SIZE; j++)
    {
        myPitch.points[j][0].status = myPitch.points[j][PITCH_Y_SIZE + 1].status = NORMAL;
        myPitch.points[j][1].status = BORDER_UP;
        myPitch.points[j][PITCH_Y_SIZE].status = BORDER_DOWN;
        myPitch.points[j][1].neighbours[0] = myPitch.points[j][1].neighbours[4] =
            myPitch.points[j][PITCH_Y_SIZE].neighbours[0] = myPitch.points[j][PITCH_Y_SIZE].neighbours[4] = 1;
    }

    //Uff, teraz linie pionowe
    for(i = 0; i < PITCH_Y_SIZE; i++)
    {
        myPitch.points[0][i].status = BORDER_LEFT;
        myPitch.points[PITCH_X_SIZE - 1][i].status = BORDER_RIGHT; ///TODO: Kiedys to trzeba poprawic moze, coby sie nie wstydzic przed dziecmi
        myPitch.points[0][i].neighbours[2] = myPitch.points[0][i].neighbours[6] =
            myPitch.points[PITCH_X_SIZE - 1][i].neighbours[2] = myPitch.points[PITCH_X_SIZE - 1][i].neighbours[6] = 1;
    }

    //Teraz linia srodkowa
    for(j = 0; j < PITCH_X_SIZE; j++)
    {
        myPitch.points[j][PITCH_Y_SIZE / 2 - 1].status = TAKEN;
        myPitch.points[j][PITCH_Y_SIZE / 2 - 1].neighbours[2] = myPitch.points[j][PITCH_Y_SIZE / 2 - 1].neighbours[6] = 1;
    }

    //No i kochane bramki <3
    int startPos = PITCH_X_SIZE / 2 - GOAL_WIDTH / 2;
    for(j = startPos; j < startPos + GOAL_WIDTH; j++)
    {
        myPitch.points[j][0].status = myPitch.points[j][PITCH_Y_SIZE + 1].status = GOAL;
        //Jeszcze trzeba wstawic wolne miejsca, bo teraz nie da sie dojsc do tej bramki :P
        /*
             |--|
        --------------
        */
        myPitch.points[j][1].neighbours[0] = myPitch.points[j][1].neighbours[4] =
            myPitch.points[j][PITCH_Y_SIZE].neighbours[0] = myPitch.points[j][PITCH_Y_SIZE].neighbours[4] = 0;

    }
    //No i na koniec oczywiscie - naprawic sasiadów - brakuje nam po jednym z kazdej strony
    /*
             |--|
        -----    -----
    */
    myPitch.points[j][1].neighbours[0] = myPitch.points[j][PITCH_Y_SIZE].neighbours[4] = 1;
    //Ostatecznie - dziala!
}

/**
    estabilishNetConnection - nawiazanie polaczenia
*/
void estabilishNetConnection()
{
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
    cleaner - czyscimy.
*/
void cleaner()
{

    if(close(mySocket) == -1)
    {
    perror("CLOSE NET SOCKET ERROR.");
    exit(1);
    }

}
