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
char path[50]; //ip
char ip[30];
int port;
///TODO: POMYSLEC NAD TRYBEM VIEWERA
pthread_t ioThread;
pthread_t parent;
pid_t parentPid;
int mySocket = -1; //-1 by sprawdzic czy sie polaczylismy gdzies po prostu
struct sockaddr_in remoteAddr;
struct sockaddr_in myAddr;
struct message msg;
struct message toSend;
struct pitch myPitch;

void estabilishNetConnection();

void cleaner();
void* work(void* data);
void* io(void* data);
char* getPort(char*);
char* getIP(char*);
struct message packMessage(int type, char* msg);
void daddyHandler(int sig);
void kidHandler(int sig);

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

int main(int argc, char** argv)
{
    ///TODO: Mozna dodac parsowanie argumentow wejscia zeby zagrac od razu!
    menu();
    return 0;
}

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

void printOptions()
{
    printf("\n1. Play a game!\n");
    printf("2. Get a high score table.\n");
    printf("3. Manual :)\n");
    printf("0. Exit.\n");

}
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
void getHighScore()
{
    //Sprawdzamy czy nie jestesmy polaczeni

}

void manual()
{


}

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
        printf("Your rival: %s. Starting game...\n", call.id2);
        playGame(call); //W call2 mamy zapisane id nasze i 'tego drugiego' :P
        ///TODO: Zrobić coś jeżeli nie chcę grać z tym gościem?
    }
    else if(type)
    {
        //Nie udalo nam sie znalezc tego jedynego...
        printf("Sorry, but %s is not connected. What do you want to do?\n", call.id2);
        printf("1. Find new player\n2.Find new specific player\n3.Back to main menu.\n");
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
    //}
}

void playGame(struct serverCaller call)
{
    //Cała logika gry
    //Narysuj boisko
    //printPitch(clientCaller) - bo z tego musimy wyiagnac info o tym gdzie mamy teraz pilke

    ///TODO: Co przesyła serwer? Po prostu x, y, czy może też całe boisko?
    ///Kto będzie decydował o logice gry - klient czy serwer?
    ///Kto trzymał będzie boisko - klient czy serwer?
    printf("\nWEEEEEE.\n");
}

void drawNeighbours(struct point point)
{

}
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


void cleaner()
{

    if(close(mySocket) == -1)
    {
    perror("CLOSE NET SOCKET ERROR.");
    exit(1);
    }

}
