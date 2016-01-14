#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <stdarg.h>
#include <sys/socket.h>
#include <bluetooth/bluetooth.h>
#include <bluetooth/rfcomm.h>
#include <mqueue.h>
#include <bumblebeeTypesAndDefs.h>

#define SERV_ADDR   "64:5A:04:2C:8D:9C"     // Whatever the address of the server is

#define BY_SSH

#define MSG_ACTION  0
#define MSG_ACK     1
#define MSG_LEAD    2
#define MSG_START   3
#define MSG_STOP    4
#define MSG_WAIT    5
#define MSG_CUSTOM  6

#define TEAM_ID     1


#define MQ_NAME_BLUETOOTH "/bluetooth" 

#define MQ_SIZE 10
#define PMODE 0666 

void init_queue_bluetooth (mqd_t *mq_desc, int open_flags) {
  struct mq_attr attr;
  
  // fill attributes of the mq
  attr.mq_maxmsg = MQ_SIZE;
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

void leader () {
    mq_unlink(MQ_BLUETOOTH_NAME);
    mqd_t mqReaderBluetooth;
    init_queue_bluetooth(&mqReaderBluetooth, O_CREAT | O_RDONLY);
    int pid = fork();
    if(pid == 0){
        drive();
    }   
     
    char string[58];
    printf ("I'm the leader...\n");
    bluetooth_message msg;
    while(1){
        msg = get_msg_bluetooth(mqReaderBluetooth)
        
        // Send ACTION message
        *((uint16_t *) string) = msgId++;
        string[2] = TEAM_ID;
        string[3] = msg.next;
        string[4] = msg.action;

        string[5] = msg.angle1;          // angle 90 degree
        string[6] = msg.angle2;

        string[7] = msg.distance;          // dist 10cm

        string[8] = msg.speed1;          // speed 20mm/s
        string[9] = msg.speed2;

        write(s, string, 10);
    }
}

void follower () {
    char string[58];
    printf ("I'm a follower...\n");

    while(1){
        // Get message
        read_from_server (s, string, 58);
        char type = string[4];
        switch (type) {
            case MSG_ACTION:
                // Send ACK message
                string[3] = string[2];      // reply to sender
                string[2] = TEAM_ID;
                string[4] = MSG_ACK;

                string[5] = string[0];      // ID of the message to acknowledge
                string[6] = string[1];

                *((uint16_t *) string) = msgId++;

                string[7] = 0x00;           // status OK
            
                write(s, string, 8);
            

                break;
        case MSG_STOP:
            return;
        case MSG_LEAD:
            leader ();
            break;
        default:
            printf ("Ignoring message %d\n", type);
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
    str2ba (SERV_ADDR, &addr.rc_bdaddr);

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
            leader ();
        else
            follower ();

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
