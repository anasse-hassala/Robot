/*
 The contents of this file are subject to the "do whatever you like"-license.
 That means: Do, whatver you want, this file is under public domain. It is an
 example for ev3c. Copy it and learn from it for your project and release
 it under every license you want. ;-)
 For feedback and questions about my Files and Projects please mail me,
 Alexander Matthes (Ziz) , ziz_at_mailbox.org, http://github.com/theZiz
*/
#include "ev3c.h"

#include <stdio.h>
#include "bumblebeeTypesAndDefs.h"

void move(ev3_motor_ptr leftM, ev3_motor_ptr rightM, int time, int speed){
	
	// set speed and time
	ev3_stop_command_motor_by_name(leftM , "coast");	
	ev3_set_speed_sp( leftM, speed);
	ev3_set_time_sp( leftM, time );
		
	ev3_stop_command_motor_by_name( rightM, "coast");
	ev3_set_speed_sp( rightM, speed);
	ev3_set_time_sp( rightM, time);
	
	//start motors
	ev3_command_motor_by_name( rightM, "run-timed");
	ev3_command_motor_by_name( leftM, "run-timed");

}


void turn(ev3_motor_ptr leftM, ev3_motor_ptr rightM, int time, int degree){
	int rightSpeed, leftSpeed;

	if(degree >= 0){			
		leftSpeed = FULLTURN_SPEED;
		rightSpeed = -FULLTURN_SPEED;
	}else{
 		leftSpeed = -FULLTURN_SPEED;
		rightSpeed = FULLTURN_SPEED;
		degree = -degree;
	}
	int turn_time = time * degree/360;

	// set speed and time
	ev3_stop_command_motor_by_name(leftM , "coast");		
	ev3_set_speed_sp( leftM, leftSpeed);
	ev3_set_time_sp( leftM, turn_time );
	
	ev3_stop_command_motor_by_name( rightM, "coast");
	ev3_set_speed_sp( rightM, rightSpeed);
	ev3_set_time_sp( rightM, turn_time);

	//start motors
	ev3_command_motor_by_name( rightM, "run-timed");
	ev3_command_motor_by_name( leftM, "run-timed");

}

void grab(ev3_motor_ptr middleM){
	ev3_stop_command_motor_by_name(middleM , "coast");		
	ev3_set_speed_sp( middleM, -480);
	ev3_set_time_sp( middleM, 250 );
	ev3_command_motor_by_name( middleM, "run-timed");
}

void release(ev3_motor_ptr middleM){
	ev3_stop_command_motor_by_name(middleM , "coast");		
	ev3_set_speed_sp( middleM, 70);
	ev3_set_time_sp( middleM, 1000 );
	ev3_command_motor_by_name( middleM, "run-timed");
}

