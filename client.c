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

pthread_t ioThread;
pthread_t parent;
pid_t parentPid;
int mySocket = -1; //-1 by sprawdzic czy sie polaczylismy gdzies po prostu
struct sockaddr_in remoteAddr;
struct sockaddr_in myAddr;
struct message msg;
struct message toSend;

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
void playGame(struct clientCaller);

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

    int answer;
    label:
    printf("Do you want to:\1. Find a player ? \n2. Play with specific player ? \n0. Back to main menu?\nChoose your option: ");
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

void findPlayer(int type)
{
    struct serverCaller call;
    struct clientCaller call2;
    strcpy(call.id, id);
    call.peerSocket = mySocket;
    call.request = FIND;
    if(type)
    {
        char id2[30];
        printf("Please input second player id: ");
        scanf("%s", id2);
        strcpy(call.id2, id2);
        call.request = FIND_SPECIFIC;
    }

    printf("Sending request...");
    if(send(mySocket, &call, sizeof(struct serverCaller),0) == -1)
    {
        perror("SEND ERROR");
        exit(1);
    }

    ///TODO: Timed response!
    if(recv(mySocket, &call2, sizeof(struct clientCaller), 0) == -1)
    {
        perror("RECV ERROR");
        exit(1);
    }

    if(call2.response == ACCEPTED)
    {
        //est dobrze, mamy z kim grać
        //GRAMY!
        printf("Your rival: %s. Starting game...\n", call2.id2);
        playGame(call2); //W call2 mamy zapisane id nasze i 'tego drugiego' :P
        ///TODO: Zrobić coś jeżeli nie chcę grać z tym gościem
    }
    else
    {
        ///TODO: Zrobić coś na wypadek gdyby serwer olał sprawę
    }

}

void playGame(struct clientCaller call)
{
    //Cała logika gry
    //Narysuj boisko
    //printPitch(clientCaller) - bo z tego musimy wyiagnac info o tym gdzie mamy teraz pilke

    ///TODO: Co przesyła serwer? Po prostu x, y, czy może też całe boisko?
    ///Kto będzie decydował o logice gry - klient czy serwer?
    ///Kto trzymał będzie boisko - klient czy serwer?
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

void* work(void* data)
{
    int ownSocket = mySocket;
    sigset_t set;
    if(sigemptyset(&set) == -1)
    {
        perror("SIGEMPTYSET ERROR");
        exit(1);
    }

    if(sigaddset(&set, SIGUSR1) == -1)
    {
        perror("SIGADDSET ERROR");
        exit(1);
    }

    if(sigaddset(&set, SIGALRM) == -1)
    {
        perror("SIGADDSET ERROR");
        exit(1);
    }

    struct sigaction sigact;
    sigact.sa_mask = set;
    sigact.sa_flags = SA_RESTART;
    sigact.sa_handler = daddyHandler;
    if(sigaction(SIGUSR1, &sigact, NULL) == -1)
    {
        perror("SIGACTION ERROR");
        exit(1);
    }

    if(sigaction(SIGALRM, &sigact, NULL) == -1)
    {
        perror("SIGACTION ERROR");
        exit(1);
    }

    while(1)
    {
        label:
        if(recv(ownSocket, &msg, sizeof(struct message), 0) == -1)
        {
            if(errno == 11)
                goto label;
            perror("RECV ERROR");
            exit(1);
        }

        if(pthread_kill(ioThread, SIGUSR2) != 0)
        {
            fprintf(stderr, "PTHREAD KILL ERROR");
            exit(1);
        }
    }

}
void* io(void* data)
{
    sigset_t set;
    sigemptyset(&set);
    sigaddset(&set, SIGUSR2);

    struct sigaction sigact;
    sigact.sa_mask = set;
    sigact.sa_flags = SA_RESTART;
    sigact.sa_handler = kidHandler;
    sigaction(SIGUSR2, &sigact, NULL);
    while(1)
    {
        char buf[BUF_SIZE];
        int i;
        for(i = 0; i < BUF_SIZE; i++)
            buf[i] = '\0';
        fgets(buf, BUF_SIZE, stdin);

        if(strlen(buf) == 0)
            continue;

        //toSend = packMessage(2, buf);
        if(pthread_kill(parent, SIGUSR1) != 0)
        {
            fprintf(stderr, "PTHREAD KILL ERROR");
            exit(1);
        }
    }

}


void registerName()
{


}

void daddyHandler(int sig)
{

    //Nie to nie, wysylamy po prostu
    int ownSocket = mySocket;


}

void kidHandler(int sig)
{
    //printf("%s: %s\n", msg.name, msg.msg);
}

void cleaner()
{

    if(close(mySocket) == -1)
    {
    perror("CLOSE NET SOCKET ERROR.");
    exit(1);
    }

}
