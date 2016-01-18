#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <stdarg.h>
#include <sys/socket.h>
#include <bluetooth/bluetooth.h>
#include <bluetooth/rfcomm.h>
#include <mqueue.h>
#include "bumblebeeTypesAndDefs.h"
#include "drive.h"

//#define SERV_ADDR   "AC:7B:A1:60:16:CE"     // Whatever the address of the server is
//#define SERV_ADDR   "00:1A:7D:DA:71:06"
//#define SERV_ADDR "78:31:C1:CF:B1:49"
#define SERV_ADDR "DC:53:60:AD:61:90"
#define BY_SSH



/* to get a bluetooth message from message queue */
struct bluetooth_message get_msg_bluetooth (mqd_t mq_desc) {
  ssize_t num_bytes_received = 0;
  struct bluetooth_message bt_msg;

  //receive an int from mq
  num_bytes_received = mq_receive (mq_desc, (char *) &bt_msg, sizeof (struct bluetooth_message)
, NULL);
  if (num_bytes_received == -1)
    perror ("mq_receive failure");
  return (bt_msg);
}



void debug (const char *fmt, ...) {
    va_list argp;

#ifdef BY_SSH
    va_start (argp, fmt);

    vprintf (fmt, argp);

    va_end (argp);
#endif

}

unsigned char rank = 0;
unsigned char length = 0;
unsigned char previous = 0xFF;
unsigned char next = 0xFF;

int s;

uint16_t msgId = 0;

int read_from_server (int sock, char *buffer, size_t maxSize) {
    int bytes_read = read (sock, buffer, maxSize);

    if (bytes_read <= 0) {
        fprintf (stderr, "Server unexpectedly closed connection...\n");
        close (s);
        exit (EXIT_FAILURE);
    }

    printf ("[DEBUG] received %d bytes\n", bytes_read);

    return bytes_read;
}

void follower () {
    int status = -1;
    char string[58];
    printf ("I'm a follower...\n");
    mqd_t mqWriterFollower;
    init_queue_follower(&mqWriterFollower, O_CREAT | O_WRONLY);
    struct follower_action action;

    while(1){
        // Get message
        read_from_server (s, string, 58);
        char type = string[4];
        int delay, speed1, speed2;
        msgId =  *((uint16_t *) string);

        switch (type) {
             case MSG_ACTION:
                    action.angle =  *((uint16_t *) (string+5));
                    action.distance =  string[7];
                    
                   // speed1 = string[8];
                    //speed2 = string[9];
                    //action.speed = speed1 + 255*speed2;

                    action.speed =  *((uint16_t *) (string+8));
                    if(action.angle > 0 && action.distance > 0 && action.speed > 0){
                        action.movement = TURN_MOVE;
                    }else{
                    if(action.angle > 0 && action.angle <= 360){
                        action.movement = TURN;    
                    } 
                    else if (action.distance != 0 && action.speed != 0){
                        action.movement = MOVE;
                    }
                    }
                break;
        case MSG_STOP:
            return;
        case MSG_LEAD:
            action.movement = MODE_LEAD;
            mq_send(mqWriterFollower, (char*)&action,sizeof (struct follower_action) , 1);
            return;
        case MSG_CANCEL:
            action.movement = CANCEL;
            break;
        case MSG_WAIT:
            delay = string[5];
            sleep(delay);
            break;
        default:
            printf ("Ignoring message %d\n", type);
        }  
        status = mq_send(mqWriterFollower, (char*)&action,sizeof (struct follower_action) , 1);
        if (status == -1)
            perror("mq_send failure");
        // Send ACK message
                string[3] = string[2];      // reply to sender
                string[2] = TEAM_ID;
                string[4] = MSG_ACK;

                string[5] = string[0];      // ID of the message to acknowledge
                string[6] = string[1];

                *((uint16_t *) string) = ++msgId;

                string[7] = 0x00;           // status OK
            
                write(s, string, 8);
  
    }
}


void leader (int mode, int previous, int next) {
    mq_unlink(MQ_BLUETOOTH);
    mqd_t mqReaderBluetooth;
    init_queue_bluetooth(&mqReaderBluetooth, O_CREAT | O_RDONLY);
    
    int pid = fork();
    
    if(pid != 0){
        printf("STARTING DRIVE\n");
        drive(mode);
        return;
    }
    if(mode == MODE_FOLLOW){
        printf("FOLLOWING\n");
        follower();
    } 

    char string[58];
    printf ("I'm the leader...\n");
    struct bluetooth_message msg;
    while(1){
        printf("TRYING TO READ FROM BTQ\n");
        msg = get_msg_bluetooth(mqReaderBluetooth);        
        printf("SENDING BT MESSAGE: \n");
        // Send ACTION message
        *((uint16_t *) string) = msgId++;
        printf("Msg ID %i\n", msgId);
        string[2] = TEAM_ID;
        printf("Team ID: %c\n", TEAM_ID);
        string[3] = next;
        printf("Next ID: %c\n", next);
        string[4] = msg.action;
        printf("Action: %i\n", msg.action);

        *((uint16_t *) (string+5)) = msg.angle;          // angle 90 degree
        printf("Angle: %i\n", msg.angle);

        string[7] = msg.distance;          // dist 10cm
        printf("distnace: %i\n",msg.distance);

        *((uint16_t *) (string+8)) = msg.speed*10;
        printf("Speed: %i\n", msg.speed);

        write(s, string, 10);
    }
    
}


int main(int argc, char **argv) {
    struct sockaddr_rc addr = { 0 };
    int status;

    // allocate a socket
    s = socket(AF_BLUETOOTH, SOCK_STREAM, BTPROTO_RFCOMM);

    // set the connection parameters (who to connect to)
    addr.rc_family = AF_BLUETOOTH;
    addr.rc_channel = (uint8_t) 1;
    str2ba(SERV_ADDR, &addr.rc_bdaddr);

    // connect to server
    status = connect(s, (struct sockaddr *)&addr, sizeof(addr));

    // if connected
    if( status == 0 ) {
        char string[58];

        // Wait for START message
        read_from_server (s, string, 9);
        if (string[4] == MSG_START) {
            printf ("Received start message!\n");
            rank = (unsigned char) string[5];
            length = (unsigned char) string[6];
             previous = (unsigned char) string[7];
            next = (unsigned char) string[8];
        }
        if (rank == 0)
            leader (MODE_LEAD,previous,next);
        else
            leader (MODE_FOLLOW,previous,next);
        printf("CLOSING");
        close (s);

        sleep (5);

    } else {
        fprintf (stderr, "Failed to connect to server...\n");
        sleep (2);
        exit (EXIT_FAILURE);
    }

    close(s);
    return 0;
}
