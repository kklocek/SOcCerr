#ifndef HEADER_H_INCLUDED
#define HEADER_H_INCLUDED

#define PITCH_X_SIZE 8
#define PITCH_Y_SIZE 10
#define GOAL_WIDTH 2

enum gameStatus
{
    GAME_PLAYING,
    PLAYER_ONE_WINS,
    PLAYER_TWO_WINS,
    PLAYER_ONE_SURRENDING,
    PLAYER_TWO_SURRENDING,
    DRAW_REQUEST,
    DRAW,
    ENDED,
    DISCONNECTED
};

enum playerStatus
{
    PLAYING,
    WAITING,
    BEING
};

enum serverRequest
{
    REGISTER,
    FIND_SPECIFIC,
    FIND,
    FOUNDED, //?
    HIGH_SCORE,
    TURN

};

enum borderStatus
{
    BORDER_UP,
    BORDER_DOWN,
    BORDER_LEFT,
    BORDER_RIGHT,
};

enum pointStatus
{
    NORMAL,
    TAKEN, //Znaczy sie mozna sie odbic od niego
    GOAL
};

enum serverResponse
{
    ACCEPTED,
    NOTACCEPTED
};

struct message
{
    int x, y;
    int idOneTurn; //Czy id1 ma teraz ture
    int sock1, sock2; //Nr socketow, aby nie szukac niepotrzebnie w tablicy
    int isEnded;
    enum gameStatus status;
};

struct serverCaller
{
    char id[30];
    char id2[30];
    enum serverRequest request;
    enum serverResponse response;
    struct message msg; //Niechserwer decyduje kto ma grac pierwszy itd.

};

struct clientCaller
{
    enum serverResponse response;
    char id[30];
    char id2[30];

};

struct point
{
    enum pointStatus status;
    int neighbours[8];
    int isItBall;
    /*
    1 2 3
    0 x 4
    7 6 5
    */
};

struct pitch
{
    struct point points[PITCH_X_SIZE][PITCH_Y_SIZE + 2];///TODO: Zdefine'owac wymiary boikska i sprawdzic czy to zadziala
};

struct playerInfo
{
    char id[30];
    int sock;
    enum playerStatus status;

};
#endif // HEADER_H_INCLUDED
