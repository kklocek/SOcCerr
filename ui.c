#include "header.h"

//extern struct pitch myPitch;
//extern int currX, currY, iAmFirst, mySocket;
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
        myPitch.points[j][PITCH_Y_SIZE / 2 + 1].status = TAKEN;
        myPitch.points[j][PITCH_Y_SIZE / 2 + 1].neighbours[0] = myPitch.points[j][PITCH_Y_SIZE / 2 + 1].neighbours[4] = 1;
    }

    //Pileczka
    myPitch.points[PITCH_X_SIZE / 2][PITCH_Y_SIZE / 2 + 1].isItBall = 1;
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


