#include "motor_movement.h"
#include "sensor_readout.h"
#include "bumblebeeTypesAndDefs.h"
#include <mqueue.h>
#include <time.h>
#include <math.h>

#define SEARCH_LEFT 0
#define SEARCH_SWEEP_RIGHT 1
#define SEARCH_RETURN 2
#define SEARCH_MOVE 3
#define SEARCH_START 4

// global variables for position
int x = 0;
int y = 0;

void construct_bluetooth_msg(struct bluetooth_message *msg, char next, char action, int angle, char distance, int speed){
    msg->next = next;
    msg->action = action;
    msg->angle = angle;
    msg->distance = distance;
    msg->speed = speed;
}

//Sensor data transmission
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

//Queue for values from bluetooth message
void init_queue_bluetooth (mqd_t *mq_desc, int open_flags) {
    struct mq_attr attr;
  
    // fill attributes of the mq
    attr.mq_maxmsg = MQ_SIZE_BLUETOOTH;
    attr.mq_msgsize =  sizeof (struct bluetooth_message);
    attr.mq_flags = 0;

    // open the mq
    //*mq_desc = mq_open (MQ_NAME, open_flags);
    *mq_desc = mq_open (MQ_BLUETOOTH, open_flags, PMODE, &attr);
    if (*mq_desc == (mqd_t)-1) {
        perror("Mq opening failed");
        exit(-1);
    }
}

// Queue for speed, angle and distance
void init_queue_follower (mqd_t *mq_desc, int open_flags) {
    struct mq_attr attr;
  
    // fill attributes of the mq
    attr.mq_maxmsg = MQ_SIZE_FOLLOWER;
    attr.mq_msgsize =  sizeof (struct follower_action);
    attr.mq_flags = 0;

    // open the mq
    //*mq_desc = mq_open (MQ_NAME, open_flags);
    *mq_desc = mq_open (MQ_FOLLOWER, open_flags, PMODE, &attr);
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

struct follower_action get_msg_follower(mqd_t mq_desc) {
    ssize_t num_bytes_received = 0;
    struct follower_action bt_msg;

    //receive an int from mq
    num_bytes_received = mq_receive (mq_desc, (char *) &bt_msg, sizeof (struct follower_action)
, NULL);
    if (num_bytes_received == -1)
        perror ("mq_receive failure");
    return (bt_msg);
}

void perform_action(ev3_motor_ptr left_M, ev3_motor_ptr right_M, struct follower_action *action){
    if(action->angle != 0){
        turn(left_M, right_M,(int)FULLTURN_TIME, action->angle);
        action->angle = 0;
    }else if(action->distance != 0 && action->speed != 0){
        move(left_M, right_M, action->distance*83, action->speed);
        action->distance=0;
        action->speed=0;
    }
}

void follow(ev3_motor_ptr leftM, ev3_motor_ptr rightM){
    mqd_t mqReaderFollower;
    init_queue_follower(&mqReaderFollower, O_CREAT | O_RDONLY);
    struct follower_action action;
    int angle=0;
    //int x=0;
    //int y =0;
    while(1){
        action = get_msg_follower(mqReaderFollower);
        printf("DRIVE\nACTION: %c\nANGLE: %c\nDISTANCE: %c\nSPEED: %c\n", action.movement, action.angle, action.distance, action.speed);

            if(action.movement != CANCEL){
                while((ev3_motor_state( leftM ) & MOTOR_RUNNING) || (ev3_motor_state( rightM ) & MOTOR_RUNNING) );
            }
        switch (action.movement){
            case TURN:  
                angle += action.angle;
                angle = angle % 360;
                if (action.angle > 180)
                    turn(leftM,rightM,(int)FULLTURN_TIME,(int)action.angle - 360);
                else
                     turn(leftM,rightM,(int)FULLTURN_TIME,(int)action.angle);
                break;
            case MOVE:
               sleep(2);
                 move(leftM, rightM, (action.distance*10000/action.speed), action.speed*2);
                y += (action.distance*10000/action.speed)*cos(angle*M_PI/180)/(action.speed*208);
                x += (action.distance*10000/action.speed)*sin(angle*M_PI/180)/(action.speed*208);
                 break;
            case TURN_MOVE:
                if (action.angle > 180)
                    turn(leftM,rightM,(int)FULLTURN_TIME,(int)action.angle - 360);
                else
                    turn(leftM,rightM,(int)FULLTURN_TIME,(int)action.angle);
                while((ev3_motor_state( leftM ) & MOTOR_RUNNING) || (ev3_motor_state( rightM ) & MOTOR_RUNNING) ); 
                move(leftM, rightM,  (action.distance*1000/action.speed), action.speed*21);
                break;
            case CANCEL:
                move(leftM, rightM, 0, 0);
                break;   
            case MODE_LEAD:
                mq_unlink(MQ_FOLLOWER);
                return;
            
        }
    } 
}

void drive(int mode){
    //Loading all motors
	ev3_motor_ptr motors  = ev3_load_motors();
	ev3_motor_ptr motor   = motors;
	ev3_motor_ptr leftM   = ev3_search_motor_by_port(motors, 'B');
	ev3_motor_ptr rightM  = ev3_search_motor_by_port(motors, 'C');
	ev3_motor_ptr middleM = ev3_search_motor_by_port(motors, 'D');
    ev3_sensor_ptr sensors  = ev3_load_sensors();                                        
    ev3_sensor_ptr distanceS = ev3_search_sensor_by_port(sensors,3);                     
    // 3 compass / 1 push / 2 color / 4 distance                                         
    ev3_mode_sensor_by_name(distanceS, "US-LISTEN");
    ev3_close_sensor(distanceS);
    ev3_delete_sensors(sensors);
    while (motor)
	{
		ev3_reset_motor(motor);
		ev3_open_motor(motor);
		motor = motor->next;
	
    }
     
    if(mode == MODE_FOLLOW){
        printf("FOLLOWING");
        follow(leftM, rightM);
    }
    printf("LEADING");
    

    // initialize mq and mutex for the control/sensor module
    mq_unlink(MQ_INSTRUCTIONS);
    mqd_t mqWriterInstr, mqReaderInstr, mqWriterBluetooth;
    
    int pthread_mutex_destroy(pthread_mutex_t *instructionLock);
    pthread_mutex_t instructionLock = PTHREAD_MUTEX_INITIALIZER;

    init_queue(&mqWriterInstr, O_CREAT | O_WRONLY);
    struct threadParams controlParams = {mqWriterInstr, &instructionLock};
    // start the control/sensor module 
    pthread_t controlThread;
    pthread_create(&controlThread, NULL, control, &controlParams); 
    // initialize reader from construction mq
    init_queue(&mqReaderInstr, O_RDONLY); 
    // initialize reader from the bluetooth mq
    init_queue_bluetooth(&mqWriterBluetooth, O_WRONLY);
    
		
    struct ev3_message instMsg;
    int instruction;
    
    struct bluetooth_message bluetoothMsg;

    struct timespec t1, t2;
    int detectDistance;
    int direction = 0;
    int count = 0;
    int WISH_ANGLE;
    int searchStage = SEARCH_LEFT;
    int lastNewMsg = -1;
    int retAngle = 0 ;
    int initAngle = 0;
    int gyroAngle = 0;
	int detectedAndNotFound = 0;
    int wall_count = 0;
    int initAngle_Start = 0;
    int x_abs = 0;
    int y_abs = 0;
    int dist_cm = 0;
    int speed_cm = 83;
    int speed_mm_s = 12;
    int angle_msg = 720;
    
    pthread_mutex_unlock(&instructionLock);
    instMsg = get_instruction_msg(mqReaderInstr);
	instruction = instMsg.instruction;
	
    initAngle_Start = instMsg.initAngle % 360;
    initAngle = instMsg.initAngle % 360;

    /*turn(leftM, rightM, (int)FULLTURN_TIME, 90);  //Right turn to first wall
    WISH_ANGLE = instMsg.gyroAngle + 90;*/
    while(1){

        if(instruction == MODE_FOLLOW){
            follow(leftM, rightM);
        }

	    switch(instruction){
	        case MSG_KEEP_SEARCHING:
	            if((instMsg.gyroAngle>=WISH_ANGLE-1 && instMsg.gyroAngle <= WISH_ANGLE+1) || !((ev3_motor_state( leftM ) & MOTOR_RUNNING) || (ev3_motor_state( rightM ) & MOTOR_RUNNING) )){ 
                    dist_cm = 0;
                    //turn and search  
		            if(lastNewMsg==MSG_BALL_DET){
                        detectedAndNotFound=1;
                        //initAngle = instMsg.initAngle % 360;     
                        move(leftM,rightM,5000,-(int)MOVE_SPEED);
                        while((ev3_motor_state( leftM ) & MOTOR_RUNNING) || 
                            (ev3_motor_state( rightM ) & MOTOR_RUNNING) );
                        int angle_search_stop = instMsg.gyroAngle - initAngle_Start; 
                       
                        turn(leftM, rightM, (int)FULLTURN_TIME, -angle_search_stop); 
                        gyroAngle = (instMsg.gyroAngle - initAngle) % 360;
                        //retAngle =180 - gyroAngle;
                        printf("Current Angle = %d \n",gyroAngle);
                        printf("Init Angle = %d \n",initAngle); 
                        //y += (instMsg.dist-detectDistance)*cos(gyroAngle*M_PI/180)/2500;
                        //x += (instMsg.dist-detectDistance)*sin(gyroAngle*M_PI/180)/2500;
                        printf("\nx=%d, y=%d\n",x,y);
                        break;
                    }
                    switch(searchStage){ 
                    // sweep from -90 to 90 degrees and look for 
                    // the ball. Detection done in sensor_readout   
                        case SEARCH_LEFT: 
                            initAngle_Start = instMsg.gyroAngle;
                            turn(leftM, rightM, (int)FULLTURN_TIME, -92); 
                            WISH_ANGLE = instMsg.gyroAngle - 90;
                            searchStage = SEARCH_SWEEP_RIGHT;
                            break;
                        case SEARCH_SWEEP_RIGHT:
                            turn(leftM, rightM, (int)FULLTURN_TIME, 180);
                            WISH_ANGLE = instMsg.gyroAngle + 180;	
                            searchStage = SEARCH_RETURN;	
                            break;
                        case SEARCH_RETURN:
                            turn(leftM, rightM, (int)FULLTURN_TIME, -92);
                            WISH_ANGLE = instMsg.gyroAngle - 90;
                            searchStage = SEARCH_MOVE;
                            break;
                        case SEARCH_MOVE:
                            move(leftM, rightM, (int)2905, (int)MOVE_SPEED);//next Pos 35cm
                            //initAngle = instMsg.initAngle % 360;
                            gyroAngle = (instMsg.gyroAngle - initAngle) % 360;
                            printf(" Raw Current Angle = %d \n",instMsg.gyroAngle); 
                            printf("Init Angle = %d \n",initAngle);
                            y += 1000*2*cos(gyroAngle*M_PI/180)/2.5;
                            x += 1000*2*sin(gyroAngle*M_PI/180)/2.5;
                            printf("\n x=%d, y=%d\n" ,x,y);
                            WISH_ANGLE = -1;
                            searchStage = SEARCH_LEFT;
                            // Need to calculate the distance in cm
                            //dist_cm = 2905/speed_cm;    //Calculates the distance in cm 
                            printf("WRITING TO LUETOOTH\n");
                            construct_bluetooth_msg(&bluetoothMsg, 2, MSG_ACTION, 0, (2905/speed_cm), speed_mm_s);//Speed isnt MOVE.. but 1.2 cm/sec
                            mq_send(mqWriterBluetooth, (char*)&bluetoothMsg,sizeof (struct bluetooth_message) , 1);
                            break;
                    }// end of switch (search)
                    lastNewMsg = MSG_KEEP_SEARCHING;
                }//end of if(gyroAngle & motor_running)
                break;

            case MSG_BALL_IN_RANGE:
                // start timer
                move(leftM,rightM,500,50);
                grab(middleM);
                while ((ev3_motor_state( middleM ) & MOTOR_RUNNING) || 
                        (ev3_motor_state( leftM ) & MOTOR_RUNNING) ||
                         (ev3_motor_state( rightM ) & MOTOR_RUNNING));
                clock_gettime(CLOCK_PROCESS_CPUTIME_ID, &t2);
                // compute and print the elapsed time in millisec
                double t_ns = (double)(t2.tv_sec - t1.tv_sec) * 1.0e9 +
                    (double)(t2.tv_nsec - t1.tv_nsec);  //Nanosec
                int elapsedTime = (int)t_ns / 1.0e6;    //Millisec
                initAngle = instMsg.initAngle % 360;
                gyroAngle = (instMsg.gyroAngle - initAngle) % 360;
                retAngle =180 - gyroAngle;
                //y += 3*elapsedTime*cos(gyroAngle*M_PI/180)/2.5;
                //x += 3*elapsedTime*sin(gyroAngle*M_PI/180)/2.5;

                int angle_search_stop = instMsg.gyroAngle - initAngle_Start; 
                printf("retAngle = %d",retAngle);
                printf("Current Angle = %d \n",gyroAngle); 
                printf("\nx=%d, y=%d\n",x,y);
                move(leftM, rightM, elapsedTime,-(int)MOVE_SPEED);
                     sleep(1);
                while (
                        ((ev3_motor_state( leftM ) & MOTOR_RUNNING) ||
                         (ev3_motor_state( rightM ) & MOTOR_RUNNING)) 
                         );
                            
                turn(leftM,rightM,(int)FULLTURN_TIME,-angle_search_stop);
                     sleep(1);
                while (
                        ((ev3_motor_state( leftM ) & MOTOR_RUNNING) ||
                         (ev3_motor_state( rightM ) & MOTOR_RUNNING)) 
                         );
               
                // handling returns from different areas of the arena
                if(instMsg.gyroAngle - instMsg.initAngle >= -45 && instMsg.gyroAngle - instMsg.initAngle < 45)
                    direction = UP;
                else if(instMsg.gyroAngle - instMsg.initAngle >= 135 && instMsg.gyroAngle - instMsg.initAngle < -135)
                    direction = DOWN;
                else if(instMsg.gyroAngle - instMsg.initAngle >= 45 && instMsg.gyroAngle - instMsg.initAngle < 135)
                    direction = RIGHT;
                else
                    direction = LEFT;

                if(direction == UP && (x>-500 && x<500)){
                    
                            turn(leftM, rightM, (int)FULLTURN_TIME, -90); 
                            sleep(1);
                            while (
                            (ev3_motor_state( leftM ) & MOTOR_RUNNING) ||
                            (ev3_motor_state( rightM ) & MOTOR_RUNNING));  
                            construct_bluetooth_msg(&bluetoothMsg, 2, MSG_ACTION, (angle_msg-90)%360, 0, 0);//Speed isnt MOVE.. but 1.2 cm/sec
                            mq_send(mqWriterBluetooth, (char*)&bluetoothMsg,sizeof (struct bluetooth_message) , 1);
                            
                            move(leftM,rightM, 2150, (int)MOVE_SPEED);
                     sleep(1);
                            while (
                            (ev3_motor_state( leftM ) & MOTOR_RUNNING) ||
                            (ev3_motor_state( rightM ) & MOTOR_RUNNING));
                            construct_bluetooth_msg(&bluetoothMsg, 2, MSG_ACTION, 0, (4150/speed_cm), speed_mm_s);//Speed isnt MOVE.. but 1.2 cm/sec
                            mq_send(mqWriterBluetooth, (char*)&bluetoothMsg,sizeof (struct bluetooth_message) , 1);
                            
                            turn(leftM, rightM, (int)FULLTURN_TIME, -90); 
                     sleep(1);
                            while (
                            (ev3_motor_state( leftM ) & MOTOR_RUNNING) ||
                            (ev3_motor_state( rightM ) & MOTOR_RUNNING)); 
                            construct_bluetooth_msg(&bluetoothMsg, 2, MSG_ACTION, (angle_msg-90)%360, 0, 0);//Speed isnt MOVE.. but 1.2 cm/sec
                            mq_send(mqWriterBluetooth, (char*)&bluetoothMsg,sizeof (struct bluetooth_message) , 1);
                            

                            move(leftM,rightM, y/2, (int)MOVE_SPEED);
                     sleep(1);
                            while (
                            (ev3_motor_state( leftM ) & MOTOR_RUNNING) ||
                            (ev3_motor_state( rightM ) & MOTOR_RUNNING)); 
                            construct_bluetooth_msg(&bluetoothMsg, 2, MSG_ACTION, 0, ((y/2)/speed_cm), speed_mm_s);//Speed isnt MOVE.. but 1.2 cm/sec
                            mq_send(mqWriterBluetooth, (char*)&bluetoothMsg,sizeof (struct bluetooth_message) , 1);
                            
                            turn(leftM, rightM, (int)FULLTURN_TIME, -90); 
                     sleep(1);
                            while (
                            (ev3_motor_state( leftM ) & MOTOR_RUNNING) ||
                            (ev3_motor_state( rightM ) & MOTOR_RUNNING)); 
                            construct_bluetooth_msg(&bluetoothMsg, 2, MSG_ACTION, (angle_msg-90)%360, 0, 0);//Speed isnt MOVE.. but 1.2 cm/sec
                            mq_send(mqWriterBluetooth, (char*)&bluetoothMsg,sizeof (struct bluetooth_message) , 1);
                            

                            move(leftM,rightM, 2150, (int)MOVE_SPEED);
                     sleep(1);
                            while (
                            (ev3_motor_state( leftM ) & MOTOR_RUNNING) ||
                            (ev3_motor_state( rightM ) & MOTOR_RUNNING)) ;
                            construct_bluetooth_msg(&bluetoothMsg, 2, MSG_ACTION, 0, (4150/speed_cm), speed_mm_s);//Speed isnt MOVE.. but 1.2 cm/sec
                            mq_send(mqWriterBluetooth, (char*)&bluetoothMsg,sizeof (struct bluetooth_message) , 1);
                            
                            turn(leftM, rightM, (int)FULLTURN_TIME, 90); 
                     sleep(1);
                            while (
                            (ev3_motor_state( leftM ) & MOTOR_RUNNING) ||
                            (ev3_motor_state( rightM ) & MOTOR_RUNNING)) ; 
                            construct_bluetooth_msg(&bluetoothMsg, 2, MSG_ACTION, (angle_msg+90)%360, 0, 0);//Speed isnt MOVE.. but 1.2 cm/sec
                            mq_send(mqWriterBluetooth, (char*)&bluetoothMsg,sizeof (struct bluetooth_message) , 1);
                            
                            move(leftM,rightM, y/2, (int)MOVE_SPEED);
                     sleep(1);
                            while (
                            (ev3_motor_state( leftM ) & MOTOR_RUNNING) ||
                            (ev3_motor_state( rightM ) & MOTOR_RUNNING)) ; 
                            construct_bluetooth_msg(&bluetoothMsg, 2, MSG_ACTION, 0, ((y/2)/speed_cm), speed_mm_s);//Speed isnt MOVE.. but 1.2 cm/sec
                            mq_send(mqWriterBluetooth, (char*)&bluetoothMsg,sizeof (struct bluetooth_message) , 1);
                            
                }            
                else if(direction == UP || direction == DOWN){
                    turn(leftM, rightM, (int)FULLTURN_TIME, -93);
                     sleep(1);
                     while (
                        ((ev3_motor_state( leftM ) & MOTOR_RUNNING) ||
                         (ev3_motor_state( rightM ) & MOTOR_RUNNING)) 
                         );
                    construct_bluetooth_msg(&bluetoothMsg, 2, MSG_ACTION, (angle_msg-90)%360, 0, 0);//Speed isnt MOVE.. but 1.2 cm/sec
                            mq_send(mqWriterBluetooth, (char*)&bluetoothMsg,sizeof (struct bluetooth_message) , 1);
                            

                    move(leftM, rightM, x, (int)MOVE_SPEED);
                     sleep(1);
                        while (
                        ((ev3_motor_state( leftM ) & MOTOR_RUNNING) ||
                         (ev3_motor_state( rightM ) & MOTOR_RUNNING)) 
                         );
                    construct_bluetooth_msg(&bluetoothMsg, 2, MSG_ACTION, 0, (x/speed_cm), speed_mm_s);//Speed isnt MOVE.. but 1.2 cm/sec
                            mq_send(mqWriterBluetooth, (char*)&bluetoothMsg,sizeof (struct bluetooth_message) , 1);
                            
                    if(direction == UP){
                        turn(leftM, rightM, (int)FULLTURN_TIME, -93);
                        construct_bluetooth_msg(&bluetoothMsg, 2, MSG_ACTION, (angle_msg-90)%360, 0, 0);//Speed isnt MOVE.. but 1.2 cm/sec
                         mq_send(mqWriterBluetooth, (char*)&bluetoothMsg,sizeof (struct bluetooth_message) , 1);
                    }

                    else{
                        turn(leftM, rightM, (int)FULLTURN_TIME, 93);
                        construct_bluetooth_msg(&bluetoothMsg, 2, MSG_ACTION, (angle_msg+90)%360, 0, 0);//Speed isnt MOVE.. but 1.2 cm/sec
                        mq_send(mqWriterBluetooth, (char*)&bluetoothMsg,sizeof (struct bluetooth_message) , 1);
                    }   

                    sleep(1);
                     while (
                        ((ev3_motor_state( leftM ) & MOTOR_RUNNING) ||
                         (ev3_motor_state( rightM ) & MOTOR_RUNNING)) 
                         );
                    move(leftM,rightM, y, (int)MOVE_SPEED);
                     while (
                        ((ev3_motor_state( leftM ) & MOTOR_RUNNING) ||
                         (ev3_motor_state( rightM ) & MOTOR_RUNNING)) 
                         );
                    construct_bluetooth_msg(&bluetoothMsg, 2, MSG_ACTION, 0, (y/speed_cm), speed_mm_s);//Speed isnt MOVE.. but 1.2 cm/sec
                            mq_send(mqWriterBluetooth, (char*)&bluetoothMsg,sizeof (struct bluetooth_message) , 1);
                            
                }else { 
                    if((direction==RIGHT && x>0) || (direction==LEFT && x<0)){     
                        move(leftM, rightM, abs(x), (int)MOVE_SPEED);
                    sleep(1);
                        while (
                            ((ev3_motor_state( leftM ) & MOTOR_RUNNING) ||
                         (ev3_motor_state( rightM ) & MOTOR_RUNNING)) 
                         );
                        construct_bluetooth_msg(&bluetoothMsg, 2, MSG_ACTION, 0, (abs(x)/speed_cm), speed_mm_s);//Speed isnt MOVE.. but 1.2 cm/sec
                            mq_send(mqWriterBluetooth, (char*)&bluetoothMsg,sizeof (struct bluetooth_message) , 1);
                            
                        if(direction == LEFT){
                            turn(leftM, rightM, (int)FULLTURN_TIME, -90);
                            construct_bluetooth_msg(&bluetoothMsg, 2, MSG_ACTION, (angle_msg-90)%360, 0, 0);//Speed isnt MOVE.. but 1.2 cm/sec
                            mq_send(mqWriterBluetooth, (char*)&bluetoothMsg,sizeof (struct bluetooth_message) , 1);
                        }else{
                            turn(leftM, rightM, (int)FULLTURN_TIME, 90);
                            construct_bluetooth_msg(&bluetoothMsg, 2, MSG_ACTION, (angle_msg+90)%360, 0, 0);//Speed isnt MOVE.. but 1.2 cm/sec
                            mq_send(mqWriterBluetooth, (char*)&bluetoothMsg,sizeof (struct bluetooth_message) , 1);
                        }

                    sleep(1);
                        while (
                            (ev3_motor_state( leftM ) & MOTOR_RUNNING) ||
                            (ev3_motor_state( rightM ) & MOTOR_RUNNING));
                            
                            move(leftM,rightM, y, (int)MOVE_SPEED);
                    sleep(1);
                        while (
                            (ev3_motor_state( leftM ) & MOTOR_RUNNING) ||
                            (ev3_motor_state( rightM ) & MOTOR_RUNNING));
                            construct_bluetooth_msg(&bluetoothMsg, 2, MSG_ACTION, 0, (y/speed_cm), speed_mm_s);//Speed isnt MOVE.. but 1.2 cm/sec
                            mq_send(mqWriterBluetooth, (char*)&bluetoothMsg,sizeof (struct bluetooth_message) , 1);
                            
                    } else {
                            turn(leftM, rightM, (int)FULLTURN_TIME, -90); 
                    sleep(1);
                            while (
                            (ev3_motor_state( leftM ) & MOTOR_RUNNING) ||
                            (ev3_motor_state( rightM ) & MOTOR_RUNNING)) ; 
                            construct_bluetooth_msg(&bluetoothMsg, 2, MSG_ACTION, (angle_msg-90)%360, 0, 0);//Speed isnt MOVE.. but 1.2 cm/sec
                            mq_send(mqWriterBluetooth, (char*)&bluetoothMsg,sizeof (struct bluetooth_message) , 1);
                            

                            move(leftM,rightM, 4150, (int)MOVE_SPEED);
                    sleep(1);
                            while (
                            (ev3_motor_state( leftM ) & MOTOR_RUNNING) ||
                            (ev3_motor_state( rightM ) & MOTOR_RUNNING)) ;
                            construct_bluetooth_msg(&bluetoothMsg, 2, MSG_ACTION, 0, (4150/speed_cm), speed_mm_s);//Speed isnt MOVE.. but 1.2 cm/sec
                            mq_send(mqWriterBluetooth, (char*)&bluetoothMsg,sizeof (struct bluetooth_message) , 1);
                            
                            turn(leftM, rightM, (int)FULLTURN_TIME, -90); 
                    sleep(1);
                            while (
                            (ev3_motor_state( leftM ) & MOTOR_RUNNING) ||
                            (ev3_motor_state( rightM ) & MOTOR_RUNNING)) ; 
                            construct_bluetooth_msg(&bluetoothMsg, 2, MSG_ACTION, (angle_msg-90)%360, 0, 0);//Speed isnt MOVE.. but 1.2 cm/sec
                            mq_send(mqWriterBluetooth, (char*)&bluetoothMsg,sizeof (struct bluetooth_message) , 1);
                            

                            move(leftM,rightM, abs(x), (int)MOVE_SPEED);
                    sleep(1);
                            while (
                            (ev3_motor_state( leftM ) & MOTOR_RUNNING) ||
                            (ev3_motor_state( rightM ) & MOTOR_RUNNING)); 
                            construct_bluetooth_msg(&bluetoothMsg, 2, MSG_ACTION, 0, (abs(x)/speed_cm), speed_mm_s);//Speed isnt MOVE.. but 1.2 cm/sec
                            mq_send(mqWriterBluetooth, (char*)&bluetoothMsg,sizeof (struct bluetooth_message) , 1);
                            
                            turn(leftM, rightM, (int)FULLTURN_TIME, 90*direction); 
                    sleep(1);
                            while (
                            (ev3_motor_state( leftM ) & MOTOR_RUNNING) ||
                            (ev3_motor_state( rightM ) & MOTOR_RUNNING)) ; 
                            construct_bluetooth_msg(&bluetoothMsg, 2, MSG_ACTION, (angle_msg+90)%360, 0, 0);//Speed isnt MOVE.. but 1.2 cm/sec
                            mq_send(mqWriterBluetooth, (char*)&bluetoothMsg,sizeof (struct bluetooth_message) , 1);
                            

                            move(leftM,rightM, (abs(y)-(4150*direction)), (int)MOVE_SPEED);
                    sleep(1);
                            while (
                            (ev3_motor_state( leftM ) & MOTOR_RUNNING) ||
                            (ev3_motor_state( rightM ) & MOTOR_RUNNING)); 
                            construct_bluetooth_msg(&bluetoothMsg, 2, MSG_ACTION, 0, ((abs(y)-(4150*direction))/speed_cm), speed_mm_s);//Speed isnt MOVE.. but 1.2 cm/sec
                            mq_send(mqWriterBluetooth, (char*)&bluetoothMsg,sizeof (struct bluetooth_message) , 1);
                            
                        }            
                    }
                    move(leftM,rightM, 5000, 200);
                    sleep(1);
                            while (
                            (ev3_motor_state( leftM ) & MOTOR_RUNNING) ||
                            (ev3_motor_state( rightM ) & MOTOR_RUNNING)); 
                
                    construct_bluetooth_msg(&bluetoothMsg, 2, MSG_LEAD, 0, (abs(x)/speed_cm), speed_mm_s);//Speed isnt MOVE.. but 1.2 cm/sec
                    mq_send(mqWriterBluetooth, (char*)&bluetoothMsg,sizeof (struct bluetooth_message) , 1);
                 // Back in touchdown zone          
                release(middleM);
                break;

            case MSG_BALL_DET:
                if(lastNewMsg!=MSG_BALL_DET || !((ev3_motor_state( leftM ) & MOTOR_RUNNING) ||
                            (ev3_motor_state( rightM ) & MOTOR_RUNNING))){
                    clock_gettime(CLOCK_PROCESS_CPUTIME_ID, &t1);
                    detectDistance = instMsg.dist;
                    move(leftM,rightM,6000,(int)MOVE_SPEED);
                    printf("BALL DETECTED!!!!!!!!!!!\n");
                    //searchStage = SEARCH_LEFT; // reset searching algorithm
                    lastNewMsg = MSG_BALL_DET;
                }
               searchStage = SEARCH_LEFT;
                break;
            case MSG_ROBOT_WALL:
		        move(leftM, rightM, 4150, -(int)MOVE_SPEED);   //Backs-up 50 cm
		        while (
                	((ev3_motor_state( leftM ) & MOTOR_RUNNING) ||
                	(ev3_motor_state( rightM ) & MOTOR_RUNNING))
		        );
                construct_bluetooth_msg(&bluetoothMsg, 2, MSG_ACTION, 0, (4150/speed_cm), speed_mm_s);
                mq_send(mqWriterBluetooth, (char*)&bluetoothMsg,sizeof (struct bluetooth_message) , 1);
 


                if(gyroAngle >= -20 && gyroAngle <= 60 ){
                    y -=  4150;
                }

                else if (gyroAngle > 60 && gyroAngle <= 150){
                    x -= 4150;
                }

                else if (gyroAngle > 150 && gyroAngle <= 240){
                    y += 4250;
                }

                else x+= 4150 ; 
                

                printf("distance: %d\n", instMsg.dist);


                turn(leftM, rightM, (int)FULLTURN_TIME, -90);
                construct_bluetooth_msg(&bluetoothMsg, 2, MSG_ACTION,(angle_msg-90)%360, 0, 0);
                mq_send(mqWriterBluetooth, (char*)&bluetoothMsg,sizeof (struct bluetooth_message) , 1);

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
    mq_unlink(MQ_INSTRUCTIONS);
    return;
}
