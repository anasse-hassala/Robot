#include "ev3c.h"
#include <time.h>
#include <stdio.h>
#include <mqueue.h>

#include "bumblebeeTypesAndDefs.h"

/*int push(ev3_sensor_ptr pushS){
		ev3_update_sensor_val(pushS);
		return pushS->val_data[0].s32;
}*/

int color(ev3_sensor_ptr colorS){
	    ev3_update_sensor_val(colorS);
	    return colorS->val_data[0].s32;
}	

int compass(ev3_sensor_ptr compassS){
		ev3_update_sensor_val(compassS);
		return compassS->val_data[0].s32;
}

int distance(ev3_sensor_ptr distanceS){
		ev3_update_sensor_val(distanceS);
		return distanceS->val_data[0].s32;
}

int gyro(ev3_sensor_ptr gyroS){
		ev3_update_sensor_val(gyroS);
		return gyroS->val_data[0].s32;
}


void *control(void *vargp){
    int distanceRO, colorRO, pushRO, compassRO, status;    
	mqd_t mqWriter;
	pthread_mutex_t* instructionLock;	
	struct ev3_message msg;

	ev3_sensor_ptr sensors  = ev3_load_sensors();
	ev3_sensor_ptr gyroS     = ev3_search_sensor_by_port(sensors,2);
	ev3_sensor_ptr colorS    = ev3_search_sensor_by_port(sensors,1);
	ev3_sensor_ptr compassS  = ev3_search_sensor_by_port(sensors,4);
	ev3_sensor_ptr distanceS = ev3_search_sensor_by_port(sensors,3);
	// 3 compass / 1 push / 2 color / 4 distance
	ev3_open_sensor(gyroS);
	ev3_open_sensor(colorS);
	ev3_open_sensor(compassS); 
	ev3_open_sensor(distanceS);
    	ev3_mode_sensor_by_name(colorS, "COL-COLOR");
	ev3_mode_sensor(distanceS,0);
    struct threadParams *params;
	params = (struct threadParams*)vargp;
    mqWriter = params->mq;
    instructionLock = params->mutex;
	int stuckCounter=0, isStuck = 0;
	//int isStuck = 0;
	int ballCounter = 0;
	int distanceBeforeJump = distance(distanceS);
	int prevDistance = distance(distanceS);
	msg.initAngle = gyro(gyroS);
	clock_t begin, end;
	double time_spent;
    while(1){	            
        distanceRO = distance(distanceS);
        colorRO = color(colorS);
        msg.gyroAngle = gyro(gyroS);    
    //pushRO = push(pushS);
        //compassRO = compass(compassS);
	//printf("COL: %i, DIST: %i\t", colorRO, distanceRO);    
        
	msg.instruction = MSG_KEEP_SEARCHING;
    	//printf("DISANCE: %d, PREV: %d ||| ", distanceRO, prevDistance);
	if(distanceRO < 150){ //350
	    	ballCounter++;
	//	printf("BALL DETECTED\n");
		if (ballCounter > 20){
			begin = clock();
			msg.instruction = MSG_BALL_DET;
			ballCounter = 0;	
		}
	}else{
		prevDistance = distanceRO;
	}

	if(distanceRO > 70 && distanceRO < 85)
		stuckCounter++;

	
        if(distanceRO < BALL_IN_RANGE_DISTANCE && colorRO == COLOR_RED){
            stuckCounter = 0;
	    ballCounter = 0;
	    msg.instruction = MSG_BALL_IN_RANGE;
	    end = clock(); 
	    time_spent = (double)(end - begin) / CLOCKS_PER_SEC;
	    printf("%d\n",time_spent);
	    msg.currentAngle = compass(compassS);
	//ballCaught = 1;
       /* }else if(pushRO == 1){
	    stuckCounter = 0;
            msg = MSG_BUTTON_PUSHED;*/
        }else if(stuckCounter>150){
	    	
		msg.instruction = MSG_ROBOT_WALL; 
		ballCounter = 0;
		stuckCounter=0;
	
	}
	pthread_mutex_lock(instructionLock);
        status = mq_send(mqWriter, (char*)&msg,sizeof (struct ev3_message) , 1);
        if (status == -1)
            perror("mq_send failure");   
	if (msg.instruction == MSG_BALL_IN_RANGE) break ;
    }
    ev3_delete_sensors(sensors);
}


