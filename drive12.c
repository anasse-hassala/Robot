#include "motor_movement.h"
#include "sensor_readout.h"
#include "bumblebeeTypesAndDefs.h"
#include <mqueue.h>

#define MQ_NAME "/instructions" 
#define MQ_BLUETOOTH_NAME "/bluetooth" 

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

/* to get a bluetooth message from message queue */
struct blue_message get_bluetooth_msg (mqd_t mq_desc) {
  ssize_t num_bytes_received = 0;
  struct ev3_message data = {0, 0};

  //receive an int from mq
  num_bytes_received = mq_receive (mq_desc, (char *) &data, sizeof (struct bluetooth_message)
, NULL);
  if (num_bytes_received == -1)
    perror ("mq_receive failure");
  return (data);
}

struct bluetooth_message construct_bluetooth_msg(char next, char action, char angle1,char angle2, char distance, char speed1, char speed2){
    struct bluetooth_msg;
    bluetooth_msg.next = next;
    bluetooth_msg.action = action;
    bluetooth_msg.angle1 = angle1;
    bluetooth_msg.angle2 = angle2;
    bluetooth_msg.distance = distance;
    bluetooth_msg.speed1 = speed1;
    bluetooth_msg.speed2 = speed2;
}

void main(void){
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
    int WISH_ANGLE;
    int randomAngle = 0;
    int searchStage = SEARCH_LEFT;
	int lastNewMsg = -1;
	int ballCaught = 0;
        int retAngle = 0 ;
       // release(middleM);
	//while (ev3_motor_state( middleM ) & MOTOR_RUNNING);
	
    pthread_mutex_unlock(&instructionLock);
    msg = get_instruction_msg(mqReader);
	instruction = msg.instruction;
	


    turn(leftM, rightM, (int)FULLTURN_TIME, 360); 
    WISH_ANGLE = msg.gyroAngle + 90;

    while(1){
        //printf("unlocking\n");
                //printf("instruction revieved: %i\n", instruction);
        
	/*if(ballCaught == 1){
		printf("I HAVE THE BALL\n");
		continue;
	}*/
    printf("CURRENT ANGLE: %d WISHED ANGLE: %d\n", msg.gyroAngle, WISH_ANGLE);
	switch(instruction){
	    case MSG_KEEP_SEARCHING:
	        if((msg.gyroAngle>=WISH_ANGLE && msg.gyroAngle <= WISH_ANGLE+1) || !((ev3_motor_state( leftM ) & MOTOR_RUNNING) || (ev3_motor_state( rightM ) & MOTOR_RUNNING) )){ 
                //turn and search  
		    

		
		switch(searchStage){
			case SEARCH_LEFT:
				//randomAngle = rand() % 360 - 180;
		    		turn(leftM, rightM, (int)FULLTURN_TIME, -360); 
                    WISH_ANGLE = msg.gyroAngle - 90;		
                    searchStage = SEARCH_SWEEP_RIGHT;
				break;
			case SEARCH_SWEEP_RIGHT:
				turn(leftM, rightM, (int)FULLTURN_TIME, 360);
		        WISH_ANGLE = msg.gyroAngle + 180;	
				searchStage = SEARCH_RETURN;	
				break;
			case SEARCH_RETURN:
				turn(leftM, rightM, (int)FULLTURN_TIME, -360);
				WISH_ANGLE = msg.gyroAngle - 90;
				searchStage = SEARCH_MOVE;
				break;
            case SEARCH_MOVE:
		    		//turn(leftM, rightM, (int)FULLTURN_TIME, 60);
		    		move(leftM, rightM, (int)1000, 000);
                    WISH_ANGLE = -1;
                    searchStage = SEARCH_LEFT;
		}
		//lastNewMsg = MSG_KEEP_SEARCHING;
	      }
                //printf("searching\n");
                break;
            case MSG_BALL_IN_RANGE:
                // grab the ball
		move(leftM,rightM,500,50);
                grab(middleM);
		while (ev3_motor_state( middleM ) & MOTOR_RUNNING);
		ballCaught = 1;
                printf("\nBALL IN RANGE BITCH! START ANGLE: %i CURRENT ANG: %i\n", msg.initAngle,msg.currentAngle);
		retAngle = abs((msg.initAngle + 128)-(msg.currentAngle+ 128));
		retAngle *= 360 ;
		retAngle /=255;
		//lastNewMsg = MSG_BALL_IN_RANGE;
		turn(leftM, rightM, (int)FULLTURN_TIME, retAngle); 
			while (
                		((ev3_motor_state( leftM ) & MOTOR_RUNNING) ||
                		(ev3_motor_state( rightM ) & MOTOR_RUNNING))
			);

                   move(leftM, rightM, 5000, 200);
		//lastNewMsg = MSG_BALL_IN_RANGE;
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
			move(leftM,rightM,5000,300);
			printf("BALL DETECTED!!!!!!!!!!!\n");
            searchStage = SEARCH_LEFT;
			//lastNewMsg = MSG_BALL_DET;
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
    return;
}
