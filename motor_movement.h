#include "ev3c.h"

void move(ev3_motor_ptr leftM, ev3_motor_ptr rightM, int time, int speed);
void turn(ev3_motor_ptr leftM, ev3_motor_ptr rightM, int time, int degree);
void grab(ev3_motor_ptr middleM);
void release(ev3_motor_ptr middleM);
