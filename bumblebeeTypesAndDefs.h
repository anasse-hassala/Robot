#include <mqueue.h>
#include <pthread.h>

#define FULLTURN_SPEED 75//150
#define FULLTURN_TIME 9600//4800

#define BALL_IN_RANGE_DISTANCE 50
#define BALL_IN_RANGE_COLOR 5
#define COLOR_RED 5

#define MSG_KEEP_SEARCHING 0
#define MSG_BALL_IN_RANGE 1
#define MSG_BUTTON_PUSHED 2
#define MSG_ROBOT_WALL 3
#define MSG_BALL_DET 4
#define MSG_FIND_EDGE 5
//#define MSG_READ_COMPASS 3

struct ev3_message {
	int instruction;
	int initAngle;
	int currentAngle;
    int gyroAngle;
};

struct threadParams {
    mqd_t mq;
    pthread_mutex_t *mutex;
};

struct bluetooth_message{
    char next;
    char action;
    char angle1;
    char angle2;
    char distance;
    char speed1;
    char speed2;
};
