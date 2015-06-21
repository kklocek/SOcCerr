#include "header.h"

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
    move - Tutaj sie ruszamy, w zasadzie logika gry :P
*/
struct serverCaller move(struct serverCaller caller)
{

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
        //Sprawdzamy czy mozemy tam wejsc
        if(myPitch.points[currX][currY].neighbours[c] == 0)
        {
            myPitch.points[currX][currY].status = TAKEN;
            if(canILeave(myPitch.points[currX + dx][currY + dy]))
                myPitch.points[currX][currY].neighbours[c]= myPitch.points[currX + dx][currY + dy].neighbours[(c + 4) % 8] = 1;
            else
            {
                printf("You can't move here, try again!.\n");
                continue;
            }

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
            drawPitch();
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
