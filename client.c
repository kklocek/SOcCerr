#define clear() printf("\033[H\033[J")

#include <stdio.h>
/**
  TODO: Zmienic sterowanie - popatrz na numpad!

*/

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
#include <sys/stat.h>
#include <fcntl.h>

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

int currX, currY;

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
struct serverCaller move(struct serverCaller caller);
struct serverCaller receive();
void actualize(struct serverCaller call);
void initPitch();
void sender(struct serverCaller call);
int canILeave(struct point p);
void fill(int i, int j, int val, int status);
void printPointStatus(struct point p);

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
    playGame - glowna petla gry.
*/
void playGame(struct serverCaller call)
{
    //Cała logika gry
    initPitch();
    currX = call.msg.x;
    currY = call.msg.y;
    printf("Pitch initialized!\n");
    //drawPitch();
    do
    {
        while((iAmFirst && !call.msg.idOneTurn) || (!iAmFirst && call.msg.idOneTurn))
            call = receive();
        //printf("Received. X = %d, Y = %d.\n", call.msg.x, call.msg.y);
        ///TODO: Angielski sprawdzic
        drawPitch();
        if(iAmFirst)
            printf("Remember that you need to score a goal on upper field!\n");
        else
            printf("Remember that you need to score a goal on down field!\n");
        if(!call.msg.isEnded)
        {
            call = move(call);
            sender(call);
            //printf("Sended. X = %d, Y = %d.\n", call.msg.x, call.msg.y);
        }

    } while(!call.msg.isEnded);
    ///TODO: Ładne zakończenie :P
    ///TODO: Brakuje opcji remisu i poddania się!
    clear();
    switch(call.msg.status)
    {
        case PLAYER_ONE_WINS:
            printf("\nPlayer %s won!\n", call.id);
            break;
        case PLAYER_TWO_WINS:
            printf("\nPlayer %s won!\n", call.id2);
            break;
        default:
            printf("Huh...\n");

    }
    printf("\nIt was a good game, thank you :).\n");
}

/**
    drawPitch - rysowanie boiska
*/
void drawPitch()
{
    //clear();
    int i, j, row;
    for(j = 0; j < PITCH_Y_SIZE + 2; j++)
    {
        //Rysujemy linie boiska, teraz trzeba narysowac wiersze
        for(row = 0; row < 3; row++)
        {
            for(i = 0; i < PITCH_X_SIZE; i++)
            {
                //Matko, jakie to glupie...
                if(row == 0)
                {
                    if(myPitch.points[i][j].neighbours[1])
                        printf("\\");
                    else
                        printf(" ");

                    if(myPitch.points[i][j].neighbours[2])
                        printf("|");
                    else
                        printf(" ");

                    if(myPitch.points[i][j].neighbours[3])
                        printf("/");
                    else
                        printf(" ");
                }
                else if(row == 1)
                {
                    if(myPitch.points[i][j].neighbours[0])
                        printf("-");
                    else
                        printf(" ");

                    if(myPitch.points[i][j].isItBall)
                        printf("o");
                    else
                        printf(" ");

                    if(myPitch.points[i][j].neighbours[4])
                        printf("-");
                    else
                        printf(" ");
                }
                else
                {
                    if(myPitch.points[i][j].neighbours[7])
                        printf("/");
                    else
                        printf(" ");

                    if(myPitch.points[i][j].neighbours[6])
                        printf("|");
                    else
                        printf(" ");

                    if(myPitch.points[i][j].neighbours[5])
                        printf("\\");
                    else
                        printf(" ");
                }
            }
            printf("\n");

        }
    }

}

/**
    move - Tutaj sie ruszamy, w zasadzie logika gry :P
*/
struct serverCaller move(struct serverCaller caller)
{

    ///TODO: ZROBIC TAK ZEBY NIE MOZNA BYLO STRZELIC SAMOBOJCZEGO STRZALU
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

        //Tablica opisujaca nam jak sie zmienia x i y - indeksem bedzie nasz wybor
        c--; //Dla latwej indeksacji
        int choiceX[] = {-1, -1, 0, 1, 1, 1, 0, -1};
        int choiceY[] = {0, -1, -1, -1, 0, 1, 1, 1};
        int dx = choiceX[c];
        int dy = choiceY[c];
        //printf("dx = %d, dy = %d.\n",  dx, dy);
        //Sprawdzamy czy mozemy tam wejsc
        if(myPitch.points[currX][currY].neighbours[c] == 0)
        {
            myPitch.points[currX][currY].status = TAKEN;
            //printf("POINTS STATUS: \n");
            //printPointStatus(myPitch.points[currX][currY]);
            //printPointStatus(myPitch.points[currX + dx][currY + dy]);
            if(canILeave(myPitch.points[currX + dx][currY + dy]))
                myPitch.points[currX][currY].neighbours[c]= myPitch.points[currX + dx][currY + dy].neighbours[(c + 4) % 8] = 1;
            else
            {
                printf("You can't move here, try again!.\n");
                continue;
            }
            //printf("AFTER ALL:\n");
            //printPointStatus(myPitch.points[currX][currY]);
            //printPointStatus(myPitch.points[currX + dx][currY + dy]);
            if(myPitch.points[currX + dx][currY + dy].status == NORMAL)
            {
                myPitch.points[currX + dx][currY + dy].status = TAKEN;
                caller.msg.idOneTurn = !caller.msg.idOneTurn;
            }
            else if(myPitch.points[currX + dx][currY + dy].status == GOAL)
            {
                if(iAmFirst && currY + dy == 0)
                    caller.msg.status = PLAYER_ONE_WINS;
                else
                    caller.msg.status = PLAYER_TWO_WINS;
                caller.msg.isEnded = 1;
                caller.msg.idOneTurn = !caller.msg.idOneTurn;
            }
            //Jeszcze pileczka
            myPitch.points[currX ][currY].isItBall = 0;
            myPitch.points[currX + dx][currY + dy].isItBall = 1;
            caller.request = TURN;
            caller.msg.x = currX + dx;
            caller.msg.y = currY + dy;
            currX = caller.msg.x;
            currY = caller.msg.y;
            //Pomocniczo
            //drawPitch();
            return caller;
        }
        else
        {
            printf("You can't move there, try again!.\n");
            continue;
        }
    }
}

/**
    canILeave - czy jak wejde do danego pola to bede mogl wyjsc
*/
int canILeave(struct point p)
{
    //8 pol w tablicy, jak bedzie tylko jedno to nie mozemy stamtad wyjsc
    int i, cnt = 0;
    for(i = 0; i < 8; i++)
    {
        if(p.neighbours[i] == 0)
            cnt++;
    }

    return cnt > 1;
}

/**
    fill - wypelniamy sasiadow punktu, gdy nasz punkt ma specjalny status (jest granica np.)
*/
void fill(int i, int j, int val, int status)
{
    int start = 0;

    switch(status)
    {
        case BORDER_UP:
            break;
        case BORDER_DOWN:
            start = 4;
            break;
        case BORDER_LEFT:
            start = 6;
            break;
        case BORDER_RIGHT:
            start = 2;
            break;
        default:
            break;
    }

    /*
        Dlaczego start + 5? Wyobraz sobie punkt ktory jest gorna granica - musi on miec tylko odblokowane u dolu, wiec
        trzeba zablokowac gorne:
        \|/
        -P-
    */
    int p;
    for(p = start; p < start + 5; p++)
        myPitch.points[i][j].neighbours[p % 8] = val;
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
    actualize - zmieniamy stan naszego boiska na podstawie wiadomosci
*/
void actualize(struct serverCaller call)
{
    myPitch.points[call.msg.x][call.msg.y].status = TAKEN;
    myPitch.points[currX][currY].isItBall = 0;
    myPitch.points[call.msg.x][call.msg.y].isItBall = 1;
    int dx = call.msg.x - currX + 1;
    int dy = call.msg.y - currY + 1; //Mamy zakres [0,2]
    //dx, dy
    int moves[3][3] = {{1, 0, 7},
                       {2, 0, 6},
                       {3, 4, 5}};

    int move = moves[dx][dy];

    myPitch.points[currX][currY].neighbours[move] = myPitch.points[call.msg.x][call.msg.y].neighbours[(move + 4) % 8] = 1;
    currX = call.msg.x;
    currY = call.msg.y;
}

/**
    initPitch - Tworzymy nasze boisko.
*/
void initPitch()
{
    //Najpierw X, potem Y
    //Najpierw puste miejsca i linie poziome
    //printf("In initpitch.\n");
    int i, j, k;
    //Czyszczenie naszych tablic w razie czego
    for(i = 0; i < PITCH_X_SIZE; i++)
    {
        for(j = 0; j < PITCH_Y_SIZE; j++)
        {
            myPitch.points[i][j].status = NORMAL;
            for(k = 0; k < 8; k++)
            {
                myPitch.points[i][j].neighbours[k] = 0;
                myPitch.points[i][j].isItBall = 0;
            }
        }
    }

    for(j = 0; j < PITCH_X_SIZE; j++)
    {
        //printf("In initpitch, x = %d.\n", j);
        myPitch.points[j][0].status = myPitch.points[j][PITCH_Y_SIZE + 1].status = NORMAL;
        myPitch.points[j][1].status = TAKEN;
        myPitch.points[j][PITCH_Y_SIZE].status = TAKEN;
        fill(j, 1, 1, BORDER_UP);
        fill(j, PITCH_Y_SIZE, 1, BORDER_DOWN);
    }

    //Uff, teraz linie pionowe
    for(i = 0; i < PITCH_Y_SIZE; i++)
    {
        myPitch.points[0][i].status = TAKEN;
        myPitch.points[PITCH_X_SIZE - 1][i].status = TAKEN;
        fill(0,i, 1, BORDER_LEFT);
        fill(PITCH_X_SIZE - 1, i, 1, BORDER_RIGHT);
    }

    //Teraz linia srodkowa
    for(j = 0; j < PITCH_X_SIZE; j++)
    {
        myPitch.points[j][PITCH_Y_SIZE / 2 - 1].status = TAKEN;
        myPitch.points[j][PITCH_Y_SIZE / 2 - 1].neighbours[0] = myPitch.points[j][PITCH_Y_SIZE / 2 - 1].neighbours[4] = 1;
    }

    //Pileczka
    myPitch.points[PITCH_X_SIZE / 2][PITCH_Y_SIZE / 2 - 1].isItBall = 1;
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
        fill(j, 1, 0, BORDER_UP);
        fill(j, PITCH_Y_SIZE, 0, BORDER_DOWN);

    }
    //No i na koniec oczywiscie - naprawic sasiadów - brakuje nam po jednym z kazdej strony
    /*
             |--|
        -----    -----
    */
    myPitch.points[startPos][1].neighbours[0] = myPitch.points[startPos + GOAL_WIDTH][PITCH_Y_SIZE].neighbours[4] = 1;
    //Ostatecznie - dziala!
}

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
