#define FULLTURN_SPEED 55
#define FULLTURN_TIME 13500
#define BALL_IN_RANGE_DISTANCE 	60
#define BALL_IN_RANGE_COLOR	8


#include "ev3c.h"

#include <stdio.h>


void move(ev3_motor_ptr leftM, ev3_motor_ptr rightM, int time, int speed){
	
	// set speed and time
	ev3_stop_command_motor_by_name(leftM , "coast");	
	ev3_set_speed_sp( leftM, speed);
	ev3_set_time_sp( leftM, time );
		
	ev3_stop_command_motor_by_name( rightM, "coast");
	ev3_set_speed_sp( rightM, speed);
	ev3_set_time_sp( rightM, time);
	
	// start motors
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
	ev3_set_speed_sp( middleM, -150);
	ev3_set_time_sp( middleM, 500 );
	ev3_command_motor_by_name( middleM, "run-timed");
}

void release(ev3_motor_ptr middleM){
	ev3_stop_command_motor_by_name(middleM , "coast");		
	ev3_set_speed_sp( middleM, 150);
	ev3_set_time_sp( middleM, 500 );
	ev3_command_motor_by_name( middleM, "run-timed");
}






int push(ev3_sensor_ptr pushS,int time){
	int i,pushV;
//	for (i = 0; i < time; i++)
//	{
		//sensor = sensors;
		//while (sensor)
		//{
			ev3_update_sensor_bin(pushS);
			ev3_update_sensor_val(pushS);
			//printf("%s [%i]: ",pushS->driver_name,pushS->port);
			int j;
			for (j = 0; j < pushS->data_count;j++)
				pushV = pushS->val_data[0].s32;
				return(pushV);
				//printf("%i ",pushS->val_data[0].s32);
				//printf("\tvalue %i: %i (raw) - %i (formated)\n",j,pushS->bin_data[0].s32,pushS->val_data[0].s32);
		//	sensor = sensor->next;
		//}
		//sleep(.1);
//	}
}

int color(ev3_sensor_ptr colorS,int time){
		int i,colorV;
//	for (i = 0; i < time; i++)
//	{
			ev3_update_sensor_bin(colorS);
			ev3_update_sensor_val(colorS);
			//printf("%s [%i]: ",sensor->driver_name,sensor->port);
			int j;
			for (j = 0; j < colorS->data_count;j++)
				colorV = colorS->val_data[0].s32;
				return(colorV);
				//printf("\tCOLOR:  %i (val)\n",colorS->val_data[0].s32);
				//printf("\tCOLOR:  %i (bin) - %i (val)",j,colorS->bin_data[0].s32,colorS->val_data[0].s32);
				//return(colorV);
//		sleep(.1);
//	}

}

int compass(ev3_sensor_ptr compassS,int time){
		int i,compassV;
//	for (i = 0; i < time; i++)
//	{
			ev3_update_sensor_bin(compassS);
			ev3_update_sensor_val(compassS);
			//printf("%s [%i]: ",compassS->driver_name,compassS->port);
			int j;
			for (j = 0; j < compassS->data_count;j++)
				compassV = compassS->val_data[0].s32;
				return(compassV);
				//printf("\tCOMPASS:  %i (bin) - %i (val)\n",j,compassS->bin_data[0].s32,compassS->val_data[0].s32);
//		sleep(.1);
//	}

}

int distance(ev3_sensor_ptr distanceS,int time){
		int i,distanceV;
	//for (i = 0; i < time; i++)
	//{
			ev3_update_sensor_bin(distanceS);
			ev3_update_sensor_val(distanceS);
			//printf("%s [%i]: ",sensor->driver_name,sensor->port);
			int j;
			for (j = 0; j < distanceS->data_count;j++)
				distanceV = distanceS->val_data[0].s32;
				return(distanceV);
				//printf("\tDIST: %i (bin) - %i (val)\n",j,distanceS->bin_data[0].s32,distanceS->val_data[0].s32);
//		sleep(.1);
	//}

}

int main(int argc,char** argv)
{
	//Loading all sensors
	ev3_sensor_ptr sensors  = ev3_load_sensors();
	ev3_sensor_ptr colorS    = ev3_search_sensor_by_port(sensors,1);
	ev3_sensor_ptr compassS  = ev3_search_sensor_by_port(sensors,4);
	ev3_sensor_ptr distanceS = ev3_search_sensor_by_port(sensors,3);
	// 3 compass / 1 push / 2 color / 4 distance
	
	//Interating over the sensors and printing some interesting data
	//ev3_sensor_ptr sensor = sensors;
	int i,time,speed,op_cl;
	//Loading all motors
	ev3_motor_ptr motors = ev3_load_motors();
	//Interating over the motors and printing some interesting data

	ev3_motor_ptr motor = motors;
	//ev3_motor_ptr sensor = sensors;
	

	ev3_motor_ptr leftM   = ev3_search_motor_by_port(motors, 'B');
	ev3_motor_ptr rightM  = ev3_search_motor_by_port(motors, 'C');
	ev3_motor_ptr middleM = ev3_search_motor_by_port(motors, 'D');

	while (motor)
	{
		ev3_reset_motor(motor);
		ev3_open_motor(motor);
		motor = motor->next;
		
		
	}
	

	//
	//ev3_sensor_ptr sensor = pushS;
	ev3_open_sensor(colorS);
	ev3_open_sensor(compassS); 
	ev3_open_sensor(distanceS);
	time = 1000;
	//ev3_mode_sensor(colorS, COL-REFLECT);	

	int t,DD;
//	printf("ABSOLUTE: %i, bool: %i \n", abs(DD - distance(distanceS,time)),	);	

	move(leftM, rightM, 15000, 100);	
	int pushV,colorV,compassV,distanceV;
	//Waiting for the second motor to get to holding position
	while (
		((ev3_motor_state( leftM ) & MOTOR_RUNNING) ||
		(ev3_motor_state( rightM ) & MOTOR_RUNNING) ||
		(ev3_motor_state( middleM ) & MOTOR_RUNNING)) && 
		color(colorS, time) < BALL_IN_RANGE_COLOR
		
	){
        //int j;
                        //for (j = 0; j < pushS->data_count;j++)
//        colorV    = color(colorS,time);
  //              printf("COL: %i \n",colorV);

    //    compassV  = compass(compassS,time);
      //          printf("COMP: %i \n",compassV);
        distanceV = distance(distanceS,time);
                printf("DIST: %i #",distanceV);
        
        };

	
	ev3_delete_motors(motors);
	ev3_delete_sensors(sensors);
	return 0;
	
	
}
