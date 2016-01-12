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

int main(int argc,char** argv)
{
	//Loading all motors
	ev3_motor_ptr motor = ev3_load_motors();
	//Loading all sensors
        ev3_sensor_ptr sensor = ev3_load_sensors();
	//Open the first motor and sensor
    	ev3_open_motor(motor);
    	ev3_open_sensor(sensor);
	//Set stop command
	ev3_stop_command_motor_by_name( motor, "coast");
	//Set motor duty cycle
	ev3_set_duty_cycle_sp( motor, 50 );
	//Set motor position
	ev3_set_position_sp( motor, 30 );
	//Set sensor mode to 0 (US-DIST-CM)
    	ev3_mode_sensor(sensor, 0);
	int i=0;
	while (i<10){
	    //Run 30 degrees relative to current position
	    ev3_command_motor_by_name( motor, "run-to-rel-pos");
	    //Wait until finished
	    while (
		(ev3_motor_state( motor) & MOTOR_RUNNING)
	    );
	    //Get new sensor value
            ev3_update_sensor_bin(sensor);
	    ev3_update_sensor_val(sensor);
	    //Print sensor value
    	    printf("%i \n", sensor->val_data[0].s32);
    	    sleep(2);
	    i++;
        }
	//Delete motors and sensors
	ev3_delete_motors(motor);
	ev3_delete_sensors(sensor);
	return 0;
}