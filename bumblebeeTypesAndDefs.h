#include <mqueue.h>
#include <pthread.h>

//MODES
#define MODE_FOLLOW 10
#define MODE_LEAD 11

// MOTOR MOVEMENT
#define FULLTURN_SPEED 72//150
#define FULLTURN_TIME 9550//4800
#define MOVE_SPEED 250

// SENSOR 
#define BALL_IN_RANGE_DISTANCE 50
#define BALL_IN_RANGE_COLOR 5
#define COLOR_RED 5
#define COLOR_BLUE 2
// LEADER/SEARCHING MESSAGES
#define MSG_KEEP_SEARCHING 0
#define MSG_BALL_IN_RANGE 1
#define MSG_BUTTON_PUSHED 2
#define MSG_ROBOT_WALL 3
#define MSG_BALL_DET 4
#define MSG_FIND_EDGE 5
//#define MSG_READ_COMPASS 3

// SERVER MESSAGES
#define MSG_ACTION  0
#define MSG_ACK     1
#define MSG_LEAD    2
#define MSG_START   3
#define MSG_STOP    4
#define MSG_WAIT    5
#define MSG_CUSTOM  6
#define MSG_CANCEL  8

#define TEAM_ID     5

// MQ VALUES
#define MQ_INSTRUCTIONS "/instructions" 
#define MQ_BLUETOOTH "/bluetooth"
#define MQ_FOLLOWER "/follower"
#define MQ_SIZE_INSTRUCTIONS 1
#define MQ_SIZE_BLUETOOTH 1
#define MQ_SIZE_FOLLOWER 1
#define PMODE 0666 

// FOLLOWER MESSAGES
#define TURN 1
#define MOVE 2
#define CANCEL 3
#define TURN_MOVE 4
// RETURN DIRECTIONS
#define UP -2
#define DOWN 2
#define RIGHT -1
#define LEFT 1

struct follower_action{
    int movement;
    int angle;
    int speed;
    int distance;
};

struct ev3_message{
	int instruction;
	int initAngle;
	int currentAngle;
    int gyroAngle;
    int dist;
};

struct threadParams {
    mqd_t mq;
    pthread_mutex_t *mutex;
};

struct bluetooth_message{
    char next;
    char action;
    uint16_t angle;
    char distance;
    uint16_t speed;
};
