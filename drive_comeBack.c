#include "motor_movement.h"
#include "sensor_readout.h"
#include "bumblebeeTypesAndDefs.h"
#include <mqueue.h>
#include <time.h>
#include <math.h>

#define MQ_NAME "/instructions" 

#define MQ_SIZE 1
#define PMODE 0666 

#define SEARCH_LEFT 0
#define SEARCH_SWEEP_RIGHT 1
#define SEARCH_RETURN 2
#define SEARCH_MOVE 3
#define SEARCH_START 4

void init_queue (mqd_t *mq_desc, int open_flags) {
  struct mq_attr attr;
  
  // fill attributes of the mq
  attr.mq_maxmsg = MQ_SIZE;
  attr.mq_msgsize =  sizeof (struct ev3_message);
  attr.mq_flags = 0;

  // open the mq
  //*mq_desc = mq_open (MQ_NAME, open_flags);
  *mq_desc = mq_open (MQ_NAME, open_flags, PMODE, &attr);
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
    mq_unlink(MQ_NAME);
    struct ev3_message msg;
    int instruction;
    mqd_t mqWriter, mqReader;
    pthread_mutex_t instructionLock;
    init_queue(&mqWriter, O_CREAT | O_WRONLY);
    // CHECK THIS LINE
    struct threadParams controlParams = {mqWriter, &instructionLock}; 
    pthread_t controlThread;
    pthread_create(&controlThread, NULL, control, &controlParams);

    init_queue(&mqReader, O_RDONLY);

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
	
	
	srand(time(NULL));
    int count = 0;
    int WISH_ANGLE;
    int randomAngle = 0;
    int searchStage = SEARCH_LEFT;
	int lastNewMsg = -1;
	int ballCaught = 0;
    int retAngle = 0 ;
    int initAngle = 0;
    int currentAngle = 0;
       // release(middleM);
	//while (ev3_motor_state( middleM ) & MOTOR_RUNNING);
	
    pthread_mutex_unlock(&instructionLock);
    msg = get_instruction_msg(mqReader);
	instruction = msg.instruction;
	


    //turn(leftM, rightM, (int)FULLTURN_TIME, 360); 
    //WISH_ANGLE = msg.gyroAngle + 90;

    while(1){
        //printf("unlocking\n");
                //printf("instruction revieved: %i\n", instruction);
        
	/*if(ballCaught == 1){
		printf("I HAVE THE BALL\n");
		continue;
	}*/
    //printf("%d\t",msg.dist);
    //printf("CURRENT ANGLE: %d WISHED ANGLE: %d\n", msg.gyroAngle, WISH_ANGLE);
	switch(instruction){
	    case MSG_KEEP_SEARCHING:
	        if((msg.gyroAngle>=WISH_ANGLE && msg.gyroAngle <= WISH_ANGLE+1) || !((ev3_motor_state( leftM ) & MOTOR_RUNNING) || (ev3_motor_state( rightM ) & MOTOR_RUNNING) )){ 
                //turn and search  
		   if(lastNewMsg==MSG_BALL_DET){
                initAngle = msg.initAngle % 360;
                currentAngle = (msg.currentAngle - initAngle) % 360;
                retAngle =180 - currentAngle;
                y += 3*5000*cos(currentAngle*M_PI/180);
                x += 3*5000*sin(currentAngle*M_PI/180);
            printf("\nx=%d, y=%d\n",x,y);
            }
		
		switch(searchStage){
			case SEARCH_LEFT:
				//randomAngle = rand() % 360 - 180;
		    		turn(leftM, rightM, (int)FULLTURN_TIME, -100); 
                    WISH_ANGLE = msg.gyroAngle - 90;		
                    searchStage = SEARCH_SWEEP_RIGHT;
				break;
			case SEARCH_SWEEP_RIGHT:
				turn(leftM, rightM, (int)FULLTURN_TIME, 190);
		        WISH_ANGLE = msg.gyroAngle + 180;	
				searchStage = SEARCH_RETURN;	
				break;
			case SEARCH_RETURN:
				turn(leftM, rightM, (int)FULLTURN_TIME, -100);
				WISH_ANGLE = msg.gyroAngle - 90;
				searchStage = SEARCH_MOVE;
				break;
            case SEARCH_MOVE:
		    		//turn(leftM, rightM, (int)FULLTURN_TIME, 60);
		    		move(leftM, rightM, (int)1000, 200);
                    initAngle = msg.initAngle % 360;
                    currentAngle = (msg.currentAngle - initAngle) % 360;
                    y +=1000*2*cos(currentAngle*M_PI/180);
                    x +=1000*2*sin(currentAngle*M_PI/180);
                    WISH_ANGLE = -1;
                    searchStage = SEARCH_LEFT;
        }
		lastNewMsg = MSG_KEEP_SEARCHING;
	      }
                //printf("searching\n");
                break;
            case MSG_BALL_IN_RANGE:

    // start timer
		move(leftM,rightM,500,50);
                grab(middleM);


    while (ev3_motor_state( middleM ) & MOTOR_RUNNING);
  
		ballCaught = 1;
                //printf("\nBALL IN RANGE BITCH! START ANGLE: %i CURRENT ANG: %i\n", msg.initAngle,msg.currentAngle);
        clock_gettime(CLOCK_PROCESS_CPUTIME_ID, &t2);

    // compute and print the elapsed time in millisec
   double t_ns = (double)(t2.tv_sec - t1.tv_sec) * 1.0e9 +
              (double)(t2.tv_nsec - t1.tv_nsec);
   int elapsedTime = (int)t_ns / 1.0e6;
    
		initAngle = msg.initAngle % 360;
        currentAngle = (msg.currentAngle - initAngle) % 360;
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

        }

        
        move(leftM,rightM,x,100);
        while (
                    ((ev3_motor_state( leftM ) & MOTOR_RUNNING) ||
                    (ev3_motor_state( rightM ) & MOTOR_RUNNING))
            );
        
        

                break;
            /*case MSG_BUTTON_PUSHED:
                // handle push
		lastNewMsg = MSG_BUTTON_PUSHED;
                printf("button pushed\n");
                break;
            */
	    case MSG_BALL_DET:
		if(lastNewMsg!=MSG_BALL_DET || !((ev3_motor_state( leftM ) & MOTOR_RUNNING) ||
                (ev3_motor_state( rightM ) & MOTOR_RUNNING))){
            clock_gettime(CLOCK_PROCESS_CPUTIME_ID, &t1);
			move(leftM,rightM,5000,300);
			printf("BALL DETECTED!!!!!!!!!!!\n");
            searchStage = SEARCH_LEFT;
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
		    WISH_ANGLE = msg.gyroAngle - 90;
            /*while (
                    	((ev3_motor_state( leftM ) & MOTOR_RUNNING) ||
                    	(ev3_motor_state( rightM ) & MOTOR_RUNNING))
		    );*/
		    //lastNewMsg = MSG_ROBOT_WALL;
		    break;
	    default:
            break;
        } 


        pthread_mutex_unlock(&instructionLock);
        msg = get_instruction_msg(mqReader);
	    instruction = msg.instruction;
	
               

    }        
    mq_unlink(MQ_NAME);
    return ;
}
