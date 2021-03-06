#define FULLTURN_SPEED 150
#define FULLTURN_TIME 4800
#define BALL_IN_RANGE_DISTANCE 	40
#define BALL_IN_RANGE_COLOR	15 //2


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
	
	//start motors
	ev3_command_motor_by_name( rightM, "run-timed");
	ev3_command_motor_by_name( leftM, "run-timed");

ev3_stop_command_motor_by_name(leftM , "brake");
ev3_stop_command_motor_by_name(rightM , "brake");

}


void turn(ev3_motor_ptr leftM, ev3_motor_ptr rightM, int time, int degree){
	
	int flip;	
	int turn_time = time * abs(degree)/360;
	if (degree >= 0){
	flip = 1;
	}
	else{
	flip = -1;	
	}
	
	// set speed and time
	ev3_stop_command_motor_by_name(leftM , "coast");		
	ev3_set_speed_sp( leftM, flip*FULLTURN_SPEED);
	ev3_set_time_sp( leftM, turn_time );
	
	ev3_stop_command_motor_by_name( rightM, "coast");
	ev3_set_speed_sp( rightM, -FULLTURN_SPEED*flip);
	ev3_set_time_sp( rightM, turn_time);

	//start motors
	ev3_command_motor_by_name( rightM, "run-timed");
	ev3_command_motor_by_name( leftM, "run-timed");
ev3_stop_command_motor_by_name(leftM , "brake");
ev3_stop_command_motor_by_name(rightM , "brake");

}

int grap(ev3_motor_ptr middleM){
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
			for (j = 0; j < compassS->data_count;j++){
				compassV = compassS->val_data[0].s32;
				return(compassV);}
				//printf("\tCOMPASS:  %i (bin) - %i (val)\n",j,compassS->bin_data[0].s32,compassS->val_data[0].s32);
//		sleep(.1);
//	}

}

int distance(ev3_sensor_ptr distanceS,int time){
		int i,distanceV;
	for (i = 0; i < time; i++)
	{
			ev3_update_sensor_bin(distanceS);
			ev3_update_sensor_val(distanceS);
			//printf("%s [%i]: ",sensor->driver_name,sensor->port);
			int j;
			for (j = 0; j < distanceS->data_count;j++)
				distanceV = distanceS->val_data[0].s32;
				return(distanceV);
				printf("\tDIST: %i (bin) - %i (val)\n",j,distanceS->bin_data[0].s32,distanceS->val_data[0].s32);
	}

}

int main(int argc,char** argv)
{
	//Loading all sensors
	ev3_sensor_ptr sensors  = ev3_load_sensors();
	ev3_sensor_ptr pushS     = ev3_search_sensor_by_port(sensors,1);
	ev3_sensor_ptr colorS    = ev3_search_sensor_by_port(sensors,2);
	ev3_sensor_ptr compassS  = ev3_search_sensor_by_port(sensors,3);
	ev3_sensor_ptr distanceS = ev3_search_sensor_by_port(sensors,4);
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
	ev3_open_sensor(pushS);
	ev3_open_sensor(colorS);
	ev3_open_sensor(compassS); 
	ev3_open_sensor(distanceS);
	time = 1000;
	
/*	
	int pushV,colorV,compassV,distanceV;	
	for (i=0;i<10000;i++){
	pushV     = push(pushS,time);
	//int j;
			//for (j = 0; j < pushS->data_count;j++)
		printf("PUSH: %i \n",pushV);

	colorV    = color(colorS,time);
		printf("COL: %i \n",colorV);

	compassV  = compass(compassS,time);
		printf("COMP: %i \n",compassV);
	distanceV = distance(distanceS,time);
		printf("DIST: %i \n\n",distanceV);
	sleep(1);	
	}
*/


	
	//distance(distanceS,time);

	/*
	for (i = 0; i < 1000; i++)
	{
		//sensor = sensors;
		//while (sensor)
		//{
			ev3_update_sensor_bin(sensor);
			ev3_update_sensor_val(sensor);
			printf("%s [%i]: ",sensor->driver_name,sensor->port);
			int j;
			for (j = 0; j < sensor->data_count;j++)
				printf("\tvalue %i: %i (raw) - %i (formated)\n",j,sensor->bin_data[0].s32,sensor->val_data[0].s32);
		//	sensor = sensor->next;
		//}
		sleep(.1);
	}
	
	*/

	//grab(middleM);
	//turn(leftM, rightM, (int)FULLTURN_TIME, 45);
	release(middleM);
	//move(leftM, rightM, 4150, 250); //Exactly 50 cm
	//move(leftM, rightM, 100000, 100);  //for the first streight walk
	move(leftM, rightM, 100, -300);  //for the first streight walk
	//move(leftM, rightM, 19500,350);   //To check the second box long walk
	//move(leftM, rightM, 6700,300);   //long side second to third
	//move(leftM,rightM, 2000, 350);
	//move(leftM,rightM, 13000, 350);


	int pushV,colorV,compassV,distanceV;
	int grap_check = 0;
	//Waiting for the second motor to get to holding position
	while (
		((ev3_motor_state( leftM ) & MOTOR_RUNNING) ||
		(ev3_motor_state( rightM ) & MOTOR_RUNNING) ||
		(ev3_motor_state( middleM ) & MOTOR_RUNNING)) && 
		color(colorS, time) < BALL_IN_RANGE_COLOR &&
		push(pushS, time) == 0 
	){
        //pushV     = push(pushS,time);
        //int j;
                        //for (j = 0; j < pushS->data_count;j++)
        //        printf("PUSH: %i \n",pushV);

        //colorV    = color(colorS,time);
          //      printf("COL: %i \n",colorV);

        //compassV  = compass(compassS,time);
                //printf("COMP: %i \n",compassV);
        distanceV = distance(distanceS,time);
                //printf("DIST: %i \n\n",distanceV);
        }




//	move(leftM, rightM, 0, 0);
 //pushV     = push(pushS,time);
        //int j;
                        //for (j = 0; j < pushS->data_count;j++)
                //printf("PUSH: %i \n",pushV);

//	MOVE FORWARD IN BASE
	

printf("Move forward");	
	move(leftM, rightM, 3500, 200);
        while (
                ((ev3_motor_state( leftM ) & MOTOR_RUNNING) ||
                (ev3_motor_state( rightM ) & MOTOR_RUNNING)) &&
		color(colorS, time) < BALL_IN_RANGE_COLOR 

        ){
     distanceV = distance(distanceS,time);
                //printf("DIST: %i \n\n",distanceV);
        }



// TURN TOWARDS 1 BALL POS

printf("Turn Right\n");
	turn(leftM, rightM,(int)FULLTURN_TIME, 105);
while (
                ((ev3_motor_state( leftM ) & MOTOR_RUNNING) ||
                (ev3_motor_state( rightM ) & MOTOR_RUNNING)) 
);
	

// WALKS TO 1 POS

printf("Go forward\n");
	move(leftM, rightM, 4150, 250); //Exactly 50 cm
	//move(leftM, rightM, 7500, 200);
	while (
		((ev3_motor_state( leftM ) & MOTOR_RUNNING) ||
		(ev3_motor_state( rightM ) & MOTOR_RUNNING)) &&
		(color(colorS, time) < BALL_IN_RANGE_COLOR) 
	);
	if (color(colorS, time) >= BALL_IN_RANGE_COLOR) {
	printf("Grap");
	grap(middleM);
 	while (
                (ev3_motor_state( middleM ) & MOTOR_RUNNING)
        );
	printf("Found ball\n");
	
	// Return to base from 1 pos 
	turn(leftM, rightM,(int)FULLTURN_TIME, -200);
while (
                ((ev3_motor_state( leftM ) & MOTOR_RUNNING) ||
                (ev3_motor_state( rightM ) & MOTOR_RUNNING)) 
);

	move(leftM, rightM, 4150, 250); //Exactly 50 cm
	//move(leftM, rightM, 7500, 200);
	while (
		((ev3_motor_state( leftM ) & MOTOR_RUNNING) ||
		(ev3_motor_state( rightM ) & MOTOR_RUNNING)) 
	);
	
	turn(leftM, rightM,(int)FULLTURN_TIME, -100);
while (
                ((ev3_motor_state( leftM ) & MOTOR_RUNNING) ||
                (ev3_motor_state( rightM ) & MOTOR_RUNNING)) 
);
	
	move(leftM, rightM, 3500, 200);
        while (
                ((ev3_motor_state( leftM ) & MOTOR_RUNNING) ||
                (ev3_motor_state( rightM ) & MOTOR_RUNNING)) 
        );

goto found_ball;
	}


// TURNS AT 1 POS

printf("Turn left\n");
	turn(leftM, rightM,(int)FULLTURN_TIME, -100);	
while (
                ((ev3_motor_state( leftM ) & MOTOR_RUNNING) ||
                (ev3_motor_state( rightM ) & MOTOR_RUNNING)) 
);



// MOVE TO 2 POS

printf("Go long side \n");
        move(leftM,rightM,30000,200);
	
	while (
		((ev3_motor_state( leftM ) & MOTOR_RUNNING) ||
		(ev3_motor_state( rightM ) & MOTOR_RUNNING)) &&
		(color(colorS, time) < BALL_IN_RANGE_COLOR) 
	){
     distanceV = distance(distanceS,time);
                //printf("DIST: %i \n\n",distanceV);
        }
	if (color(colorS, time) >= BALL_IN_RANGE_COLOR) {
	printf("Grap\n");
	grap(middleM);
 	while (
                (ev3_motor_state( middleM ) & MOTOR_RUNNING)
        );

	// Return from 2 Pos to home base
	turn(leftM, rightM,(int)FULLTURN_TIME, -96);
while (
                ((ev3_motor_state( leftM ) & MOTOR_RUNNING) ||
                (ev3_motor_state( rightM ) & MOTOR_RUNNING)) 
);
	move(leftM, rightM, 4150, 250); //Exactly 50 cm
	//move(leftM, rightM, 7500, 200);
        while (
                ((ev3_motor_state( leftM ) & MOTOR_RUNNING) ||
                (ev3_motor_state( rightM ) & MOTOR_RUNNING)) 
        );
	turn(leftM, rightM,(int)FULLTURN_TIME, -96);
while (
                ((ev3_motor_state( leftM ) & MOTOR_RUNNING) ||
                (ev3_motor_state( rightM ) & MOTOR_RUNNING)) 
);
	move(leftM, rightM, 24900, 250); //Exactly 50 cm
	//move(leftM, rightM, (19500+1300), 350);
        while (
                ((ev3_motor_state( leftM ) & MOTOR_RUNNING) ||
                (ev3_motor_state( rightM ) & MOTOR_RUNNING)) 
        );


goto found_ball;	
	}

// TURN AT 2 POS

	turn(leftM, rightM,(int)FULLTURN_TIME, -96);	
while (
                ((ev3_motor_state( leftM ) & MOTOR_RUNNING) ||
                (ev3_motor_state( rightM ) & MOTOR_RUNNING)) 
);



// MOVE TO 3 POS

	move(leftM, rightM, 8300, 250); //Exactly 50 cm
	//move(leftM, rightM, 6700,300);   //long side second to third
	
	while (
		((ev3_motor_state( leftM ) & MOTOR_RUNNING) ||
		(ev3_motor_state( rightM ) & MOTOR_RUNNING)) &&
		(color(colorS, time) < BALL_IN_RANGE_COLOR) 	
	);
			
	if (color(colorS, time) >= BALL_IN_RANGE_COLOR) {
	printf("Grap\n");
	grap(middleM);
 	while (
                (ev3_motor_state( middleM ) & MOTOR_RUNNING)
        );

	// Return from 3 Pos to homebase
	turn(leftM, rightM,(int)FULLTURN_TIME, -100);
while (
                ((ev3_motor_state( leftM ) & MOTOR_RUNNING) ||
                (ev3_motor_state( rightM ) & MOTOR_RUNNING)) 
);
	move(leftM, rightM, (6*4150), 250); //Exactly 50 cm
	//move(leftM, rightM, 19500, 350);
        while (
                ((ev3_motor_state( leftM ) & MOTOR_RUNNING) ||
                (ev3_motor_state( rightM ) & MOTOR_RUNNING)) 
        );
	turn(leftM, rightM,(int)FULLTURN_TIME, -100);
while (
                ((ev3_motor_state( leftM ) & MOTOR_RUNNING) ||
                (ev3_motor_state( rightM ) & MOTOR_RUNNING)) 
);
	move(leftM, rightM, 4150, 250); //Exactly 50 cm
	//move(leftM, rightM, 7500, 200);
        while (
                ((ev3_motor_state( leftM ) & MOTOR_RUNNING) ||
                (ev3_motor_state( rightM ) & MOTOR_RUNNING)) 
        );
	turn(leftM, rightM,(int)FULLTURN_TIME, 100);
while (
                ((ev3_motor_state( leftM ) & MOTOR_RUNNING) ||
                (ev3_motor_state( rightM ) & MOTOR_RUNNING)) 
);
	move(leftM, rightM, (1750), 200);
        while (
                ((ev3_motor_state( leftM ) & MOTOR_RUNNING) ||
                (ev3_motor_state( rightM ) & MOTOR_RUNNING)) 
        );


goto found_ball;	
	}

// TURN AT 3 POS

	turn(leftM, rightM,(int)FULLTURN_TIME, -103);	
while (
                ((ev3_motor_state( leftM ) & MOTOR_RUNNING) ||
                (ev3_motor_state( rightM ) & MOTOR_RUNNING)) 
);
	
// MOVE TO 4 POS

	move(leftM, rightM, (6*4150), 250); //Exactly 50 cm
	//move(leftM,rightM, 2000, 350);
	while (
		((ev3_motor_state( leftM ) & MOTOR_RUNNING) ||
		(ev3_motor_state( rightM ) & MOTOR_RUNNING)) &&
		(color(colorS, time) < BALL_IN_RANGE_COLOR )	
	);
	
	if (color(colorS, time) >= BALL_IN_RANGE_COLOR) {
	printf("Grap\n");
	grap(middleM);
 	while (
                (ev3_motor_state( middleM ) & MOTOR_RUNNING)
        );

	// Return from 4 Pos to home base
	turn(leftM, rightM,(int)FULLTURN_TIME, -100);
while (
                ((ev3_motor_state( leftM ) & MOTOR_RUNNING) ||
                (ev3_motor_state( rightM ) & MOTOR_RUNNING)) 
);
	//move(leftM, rightM, 4150, 250); //Exactly 50 cm
	move(leftM, rightM, 7500, 200);
        while (
                ((ev3_motor_state( leftM ) & MOTOR_RUNNING) ||
                (ev3_motor_state( rightM ) & MOTOR_RUNNING)) 
        );
	turn(leftM, rightM,(int)FULLTURN_TIME, 100);
while (
                ((ev3_motor_state( leftM ) & MOTOR_RUNNING) ||
                (ev3_motor_state( rightM ) & MOTOR_RUNNING)) 
);
	move(leftM, rightM, (1750), 200);
        while (
                ((ev3_motor_state( leftM ) & MOTOR_RUNNING) ||
                (ev3_motor_state( rightM ) & MOTOR_RUNNING)) 
        );


goto found_ball;	
	}

printf("Stop\n");
	release(middleM);
	while (
		(ev3_motor_state( middleM ) & MOTOR_RUNNING)
	);
	found_ball: printf("Tobi just found some ball\n");			
	//move
	//Let's delete the list in the very end. It will also close the
	//motors

	ev3_delete_motors(motors);
	ev3_delete_sensors(sensors);
	return 0;
	
	
}

