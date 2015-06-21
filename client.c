#include "header.h"
int mySocket = -1; //-1 by sprawdzic czy sie polaczylismy gdzies po prostu
int iAmFirst = 0; //Czy jestem graczem nr 1

char id[30];
char ip[30];
int port = 0;
///TODO: POMYSLEC NAD TRYBEM VIEWERA

pthread_t ioThread;
pthread_t parent;
pid_t parentPid;
struct sockaddr_in remoteAddr;
struct sockaddr_in myAddr;
struct pitch myPitch;

int currX = 0, currY = 0;

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
    prepareToGame - ustanawia polaczenie i tworzy naszego gracza.
*/
void prepareToGame()
{
    ///TODO: Zapytanie czy z tym samym serwerem - może być tak że jestem cały czas połączony, chce sobie poczytać mana, a potem zagrać jeszcze raz
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
    clear();
    FILE* fd;
    fd = fopen("README.md", "r");
    if(fd == NULL)
    {
        perror("FOPEN ERROR IN MANUAL");
        exit(1);
    }

    char buf[100];

    while(fgets(buf, 100, fd)!= NULL)
    {
        //fread(buf, 1, 100, fd); //BŁĄD!
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

    if(close(mySocket) == -1)
    {
    perror("CLOSE NET SOCKET ERROR.");
    exit(1);
    }

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
