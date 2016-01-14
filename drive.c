#include "motor_movement.h"
#include "sensor_readout.h"
#include "bumblebeeTypesAndDefs.h"
#include <mqueue.h>
#include <time.h>
#include <math.h>

#define MQ_INSTRUCTIONS "/instructions" 
#define MQ_BLUETOOTH "/bluetooth"

#define MQ_SIZE_INSTRUCTIONS 1
#define MQ_SIZE_BLUETOOTH 10
#define PMODE 0666 

#define SEARCH_LEFT 0
#define SEARCH_SWEEP_RIGHT 1
#define SEARCH_RETURN 2
#define SEARCH_MOVE 3
#define SEARCH_START 4

void init_queue (mqd_t *mq_desc, int open_flags) {
  struct mq_attr attr;
  
  // fill attributes of the mq
  attr.mq_maxmsg = MQ_SIZE_INSTRUCTIONS;
  attr.mq_msgsize =  sizeof (struct ev3_message);
  attr.mq_flags = 0;

  // open the mq
  //*mq_desc = mq_open (MQ_INSTRUCTIONS, open_flags);
  *mq_desc = mq_open (MQ_INSTRUCTIONS, open_flags, PMODE, &attr);
  if (*mq_desc == (mqd_t)-1) {
    perror("Mq opening failed");
    exit(-1);
  }
}

void init_queue_bluetooth (mqd_t *mq_desc, int open_flags) {
  struct mq_attr attr;
  
  // fill attributes of the mq
  attr.mq_maxmsg = MQ_SIZE_BLUETOOTH;
  attr.mq_msgsize =  sizeof (struct bluetooth_message);
  attr.mq_flags = 0;

  // open the mq
  //*mq_desc = mq_open (MQ_NAME, open_flags);
  *mq_desc = mq_open (MQ_BLUETOOTH_NAME, open_flags, PMODE, &attr);
  if (*mq_desc == (mqd_t)-1) {
    perror("Mq opening failed");
    exit(-1);
  }
}


/* to get an integer from message queue */
struct ev3_message get_instruction_msg (mqd_t mq_desc) {
  ssize_t num_bytes_received = 0;
  struct ev3_message data = {0, 0};

  //receive an int from mq
  num_bytes_received = mq_receive (mq_desc, (char *) &data, sizeof (struct ev3_message)
, NULL);
  if (num_bytes_received == -1)
    perror ("mq_receive failure");
  return (data);
}


void main(void){
    //Coordinates
    int x = 0;
    int y = 0;
    struct timespec t1, t2;

    // initialize mq and mutex for the control/sensor module
    mq_unlink(MQ_NAME);
    mqd_t mqWriterInstr, mqReaderInstr, mqWriterBluetooth;
    pthread_mutex_t instructionLock;
    init_queue(&mqWriterInstr, O_CREAT | O_WRONLY);
    struct threadParams controlParams = {mqWriter, &instructionLock};
    // start the control/sensor module 
    pthread_t controlThread;
    pthread_create(&controlThread, NULL, control, &controlParams); 
    init_queue(&mqReaderInstr, O_RDONLY); // set up reader 

    // initialize reader from the bluetooth mq
    init_queue_bluetooth(&mqWriterBluetooth, O_WRONLY);
    
	//Loading all motors
	ev3_motor_ptr motors = ev3_load_motors();
	ev3_motor_ptr motor = motors;
	ev3_motor_ptr leftM   = ev3_search_motor_by_port(motors, 'B');
	ev3_motor_ptr rightM  = ev3_search_motor_by_port(motors, 'C');
	ev3_motor_ptr middleM = ev3_search_motor_by_port(motors, 'D');
	while (motor)
	{
		ev3_reset_motor(motor);
		ev3_open_motor(motor);
		motor = motor->next;
	}
	
    struct ev3_message instMsg;
    int instruction;
    
    struct bluetooth_message bluetoothMsg;

    int count = 0;
    int WISH_ANGLE;
    int searchStage = SEARCH_LEFT;
    int lastNewMsg = -1; //is this useful?
    int retAngle = 0 ;
    int initAngle = 0;
    int currentAngle = 0;
	
    pthread_mutex_unlock(&instructionLock);
    instructionMsg = get_instruction_msg(mqReaderInstr);
	instruction = instructionMsg.instruction;
	

    while(1){
	    switch(instruction){
	        case MSG_KEEP_SEARCHING:
	            if((instMsg.gyroAngle>=WISH_ANGLE && instMsg.gyroAngle <= WISH_ANGLE+1) || !((ev3_motor_state( leftM ) & MOTOR_RUNNING) || (ev3_motor_state( rightM ) & MOTOR_RUNNING) )){ 
                    //turn and search  
		            if(lastNewMsg==MSG_BALL_DET){
                        initAngle = instMsg.initAngle % 360;
                        currentAngle = (instMsg.currentAngle - initAngle) % 360;
                        retAngle =180 - currentAngle;
                        y += 3*5000*cos(currentAngle*M_PI/180);
                        x += 3*5000*sin(currentAngle*M_PI/180);
                        printf("\nx=%d, y=%d\n",x,y);
                    }
		
		            switch(searchStage){
			            case SEARCH_LEFT:
		    		        turn(leftM, rightM, (int)FULLTURN_TIME, -100); 
                            WISH_ANGLE = instMsg.gyroAngle - 90;
                            searchStage = SEARCH_SWEEP_RIGHT;
				            break;
			            case SEARCH_SWEEP_RIGHT:
				            turn(leftM, rightM, (int)FULLTURN_TIME, 190);
		                    WISH_ANGLE = instMsg.gyroAngle + 180;	
				            searchStage = SEARCH_RETURN;	
				            break;
			            case SEARCH_RETURN:
				            turn(leftM, rightM, (int)FULLTURN_TIME, -100);
				            WISH_ANGLE = instMsg.gyroAngle - 90;
				            searchStage = SEARCH_MOVE;
				            break;
                        case SEARCH_MOVE:
		    		        move(leftM, rightM, (int)1000, 200);
                            initAngle = instMsg.initAngle % 360;
                            currentAngle = (instMsg.currentAngle - initAngle) % 360;
                            y +=1000*2*cos(currentAngle*M_PI/180);
                            x +=1000*2*sin(currentAngle*M_PI/180);
                            WISH_ANGLE = -1;
                            searchStage = SEARCH_LEFT;
                        break;
                    }// end of switch (search)
		        lastNewMsg = MSG_KEEP_SEARCHING;
	            }//end of if(gyroAngle & motor_running)
                break;
            case MSG_BALL_IN_RANGE:
                // start timer
		        move(leftM,rightM,500,50);
                grab(middleM);
                while (ev3_motor_state( middleM ) & MOTOR_RUNNING);
                clock_gettime(CLOCK_PROCESS_CPUTIME_ID, &t2);
                // compute and print the elapsed time in millisec
                double t_ns = (double)(t2.tv_sec - t1.tv_sec) * 1.0e9 +
                    (double)(t2.tv_nsec - t1.tv_nsec);
                int elapsedTime = (int)t_ns / 1.0e6;
		        initAngle = instMsg.initAngle % 360;
                currentAngle = (instMsg.currentAngle - initAngle) % 360;
                retAngle =180 - currentAngle;
                y += 3*elapsedTime*cos(currentAngle*M_PI/180);
                x += 3*elapsedTime*sin(currentAngle*M_PI/180); 
                printf("\nx=%d, y=%d\n",x,y);
                
                turn(leftM,rightM,(int)FULLTURN_TIME,retAngle);
                while (
                    ((ev3_motor_state( leftM ) & MOTOR_RUNNING) ||
                    (ev3_motor_state( rightM ) & MOTOR_RUNNING))
                );
                
                move(leftM,rightM,y,100);
                while (
                    ((ev3_motor_state( leftM ) & MOTOR_RUNNING) ||
                    (ev3_motor_state( rightM ) & MOTOR_RUNNING))
                );
                if(x>=0){ 
                    turn(leftM,rightM,(int)FULLTURN_TIME,90);
                    while (
                        ((ev3_motor_state( leftM ) & MOTOR_RUNNING) ||
                        (ev3_motor_state( rightM ) & MOTOR_RUNNING))
                    );
                
                    move(leftM,rightM,x,100);
                    while (
                        ((ev3_motor_state( leftM ) & MOTOR_RUNNING) ||
                        (ev3_motor_state( rightM ) & MOTOR_RUNNING))
                    );
        
                    turn(leftM,rightM,(int)FULLTURN_TIME,-90);
                    while (
                        ((ev3_motor_state( leftM ) & MOTOR_RUNNING) ||
                        (ev3_motor_state( rightM ) & MOTOR_RUNNING))
                    );

                    move(leftM,rightM,2000,200);
                    while (
                        ((ev3_motor_state( leftM ) & MOTOR_RUNNING) ||
                        (ev3_motor_state( rightM ) & MOTOR_RUNNING))
                    );
                    release(middleM);
                    return;
                }
                else{
                    turn(leftM,rightM,(int)FULLTURN_TIME,-90);
                    while (
                        ((ev3_motor_state( leftM ) & MOTOR_RUNNING) ||
                        (ev3_motor_state( rightM ) & MOTOR_RUNNING))
                    );
                    
                    move(leftM,rightM,x,100);
                    while (
                        ((ev3_motor_state( leftM ) & MOTOR_RUNNING) ||
                        (ev3_motor_state( rightM ) & MOTOR_RUNNING))
                    );

                    turn(leftM,rightM,(int)FULLTURN_TIME,90);
                    while (
                        ((ev3_motor_state( leftM ) & MOTOR_RUNNING) ||
                        (ev3_motor_state( rightM ) & MOTOR_RUNNING))
                    );

                    move(leftM,rightM,2000,200);
                    while (
                        ((ev3_motor_state( leftM ) & MOTOR_RUNNING) ||
                        (ev3_motor_state( rightM ) & MOTOR_RUNNING))
                    );
                    release(middleM);
                    return;
                }//end of if/else (x>=0)

                move(leftM,rightM,x,100);
                while (
                    ((ev3_motor_state( leftM ) & MOTOR_RUNNING) ||
                    (ev3_motor_state( rightM ) & MOTOR_RUNNING))
                );
                break;
	        case MSG_BALL_DET:
		        if(lastNewMsg!=MSG_BALL_DET || !((ev3_motor_state( leftM ) & MOTOR_RUNNING) ||
                        (ev3_motor_state( rightM ) & MOTOR_RUNNING))){
                    clock_gettime(CLOCK_PROCESS_CPUTIME_ID, &t1);
			        move(leftM,rightM,5000,300);
			        printf("BALL DETECTED!!!!!!!!!!!\n");
                    searchStage = SEARCH_LEFT; // reset searching algorithm
			        lastNewMsg = MSG_BALL_DET;
		        }
		        break;
	        case MSG_ROBOT_WALL:
		        move(leftM, rightM, 4150, -250);
		        while (
                	((ev3_motor_state( leftM ) & MOTOR_RUNNING) ||
                	(ev3_motor_state( rightM ) & MOTOR_RUNNING))
		        );
		        turn(leftM, rightM, (int)FULLTURN_TIME, -360);
		        WISH_ANGLE = instMsg.gyroAngle - 90;
		        break;
	        default:
                break;
        }//end of switch(instruction) 

        // read next instruction
        pthread_mutex_unlock(&instructionLock);
        instMsg = get_instruction_msg(mqReaderInstr);
	    instruction = instMsg.instruction;
	
    } // end of while(1)       
    mq_unlink(MQ_NAME);
    return;
}
