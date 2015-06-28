#include "header.h"
int mySocket = -1; //-1 by sprawdzic czy sie polaczylismy gdzies po prostu
int iAmFirst = -1; //Czy jestem graczem nr 1, -1 gdy nie gram

char id[30];
char ip[30];
int port = 0;

struct serverCaller call;
struct sockaddr_in remoteAddr;
struct sockaddr_in myAddr;
struct pitch myPitch;

int currX = 0, currY = 0;

/**
    main - main.
*/
int main(int argc, char** argv)
{
    struct sigaction s;
    sigset_t set;
    sigemptyset(&set);
    //sigaddset(&set, SIGSEGV);
    s.sa_flags = SA_RESTART;
    s.sa_mask = set;
    s.sa_handler = handler;
    if(sigaction(SIGINT, &s, NULL) == -1)
    {
        perror("SIGACTION ERROR");
        exit(1);
    }

    if(sigaction(SIGSEGV, &s, NULL) == -1)
    {
        perror("SIGACTION ERROR");
        exit(1);
    }

    atexit(cleaner);
    menu();
    return 0;
}



/**
    prepareToGame - ustanawia polaczenie i tworzy naszego gracza.
*/
void prepareToGame()
{
    short useIt = 0;
    if(mySocket != -1)
    {
        printf("You are connected to a server. Do you want to:\n1. Use this connection?\n2.Connect to other server?0. Back to main menu?\n");
        printf("Choice: ");
        short c;
        scanf("%hd", &c);
        if(c == 1)
            useIt = 1;
        else if(c == 2)
            useIt = 0;
        else
            menu();
    }

    if(!useIt)
    {
        printf("Input your nickname: ");
        scanf("%s", id);

        printf("Input ip: ");
        scanf("%s", ip);

        printf("Input port: ");
        scanf("%d", &port);


        estabilishNetConnection();
        registerName();
    }

    int answer;
    while(1)
    {
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
        }
    }
}

/**
    ???
*/
void getHighScore()
{
    //Sprawdzamy czy nie jestesmy polaczeni
    short useIt = 0;
    if(mySocket != -1)
    {
        printf("You are connected to a server. Do you want to:\n1. Use this connection?\n2. Connect to other server?\n0. Back to main menu?\n");
        printf("Choice: ");
        short c;
        scanf("%hd", &c);
        if(c == 1)
            useIt = 1;
        else if(c == 2)
            useIt = 0;
        else
            menu();
    }

    if(!useIt)
    {
        printf("Input your nickname: ");
        scanf("%s", id);

        printf("Input ip: ");
        scanf("%s", ip);

        printf("Input port: ");
        scanf("%d", &port);


        estabilishNetConnection();
        registerName();
    }

    struct serverCaller caller;
    strcpy(caller.id, id);
    printf("Input second id: ");
    scanf("%s", caller.id2);

    caller.request = GET_RECORD;

    if(send(mySocket, &caller, sizeof(struct serverCaller), 0) == -1)
    {
        perror("SEND HI-SCORE ERROR.");
        exit(1);
    }

    char buf[320 * 6]; //320 - max liczba ruchow
    strcpy(buf, "");
    if(recv(mySocket, buf, 320 * 6, 0) == -1)
    {
        perror("RECV HI-SCORE ERROR");
        exit(1);
    }

    if(strlen(buf) == 0)
    {
        printf("We can't find your record id. Returning to menu...\n");
        return;
    }

    FILE* myF;
    myF = fopen(id, "a");
    if(myF == NULL)
    {
        perror("FOPEN ERROR IN HI-SCORE.");
        exit(1);
    }
    printf("MOVES:\n");
    fwrite(buf, 1, strlen(buf), myF);
    fwrite(buf, 1, strlen(buf), stdout);
    if(fclose(myF) == -1)
    {
        perror("FCLOSE ERROR");
        exit(1);
    }
}

/**
    manual - wyswietla manual
*/
void manual()
{
    clear();
    FILE* fd;
    fd = fopen("README.txt", "r");
    if(fd == NULL)
    {
        perror("FOPEN ERROR IN MANUAL");
        exit(1);
    }

    char buf[100];

    while(fgets(buf, 100, fd)!= NULL)
    {
        printf("%s", buf);
    }

    if(fclose(fd) == -1)
    {
        perror("FCLOSE ERROR IN MANUAL");
        exit(1);
    }
}



/**
    cleaner - czyscimy.
*/
void cleaner()
{
    if(mySocket == -1)
	return;
    if(close(mySocket) == -1)
    {
    perror("CLOSE NET SOCKET ERROR.");
    exit(1);
    }

}

/**

*/

void handler(int sig)
{
    //Nie bylismy polaczeni
    if(mySocket == -1)
        exit(0);

    //Bylismy polaczeni, ale nie gramy
    if(iAmFirst == -1)
        exit(0);

    //Polaczeni i gramy - w takim razie przegrywamy
    int status;
    if(iAmFirst)
    {
        status = PLAYER_ONE_SURRENDING;
        call.msg.idOneTurn = 0;
    }
    else
    {
        status = PLAYER_TWO_SURRENDING;
        call.msg.idOneTurn = 0;
    }

    call.msg.isEnded = 1;
    call.msg.status = status;
    call.request = TURN;
    if(send(mySocket, &call, sizeof(struct serverCaller), 0) == -1)
    {
        perror("SEND IN SIGNAL ERROR");
        exit(1);
    }
    exit(0);
}

void printPointStatus(struct point p)
{
    int i;
    for(i = 0; i < 8; i++)
    {
        printf("%d = ", i);
        if(p.neighbours[i])
            printf("1\n");
        else
            printf("0\n");
    }

}
