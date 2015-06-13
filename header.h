#ifndef HEADER_H_INCLUDED
#define HEADER_H_INCLUDED
enum gameStatus
{
    GAME_PLAYING,
    PLAYER_ONE_WINS,
    PLAYER_TWO_WINS,
    PLAYER_ONE_SURRENDING,
    PLAYER_TWO_SURRENDING,
    DRAW_REQUEST,
    DRAW,
    DISCONNECTED
};

enum playerStatus
{
    PLAYING,
    WAITING
};

enum serverRequest
{
    REGISTER,
    FIND_SPECIFIC,
    FIND,
    FOUNDED, //?
    HIGH_SCORE

};


enum serverResponse
{
    ACCEPTED,
    NOTACCEPTED
};

struct message
{
    char id1[30];
    char id2[30];
    int peerSocket1;
    int peerSocket2;
    int x, y;
    int idOneTurn; //Czy id1 ma teraz ture
    enum gameStatus status;
};

struct serverCaller
{
    char id[30];
    char id2[30];
    int peerSocket;
    enum serverRequest request;
    struct message msg; //Niechserwer decyduje kto ma grac pierwszy itd.

};

struct clientCaller
{
    enum serverResponse response;
    char id[30];
    char id2[30];

};



struct playerInfo
{
    char id[30];
    int sock;
    enum playerStatus status;

};
///TODO: Ogarnac ponizsze
struct peerInfo
{
    char name[30];
    long timer;
    struct sockaddr addr;
    struct sockaddr_in addrIn;
    struct sockaddr_un addrUn;
    unsigned int peerSocket;
    unsigned int type;
};


#endif // HEADER_H_INCLUDED
