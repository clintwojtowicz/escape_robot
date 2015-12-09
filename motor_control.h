/*
 * motor_control.h
 *
 * Created: 11/10/2015 4:38:43 PM
 *  Author: Clint
 */ 


#ifndef MOTOR_CONTROL_H_
#define MOTOR_CONTROL_H_

#include <avr/io.h>

#define MAX_TICKS_MOTOR 10000
#define MAX_SPEED_LIMIT_TICKS 10000
#define MIN_SPEED_LIMIT_TICKS 7000
#define TICK_DELTA_MOTOR 500
#define MAX_TICKS_RAMP 20000

#define MOTOR_SLOW_TICKS 2000
#define MOTOR_MEDIUM_TICKS 5000
#define MOTOR_FAST_TICKS 8000

#define MOTOR_FORWARDS 1
#define MOTOR_BACKWARDS 0

#define LEFT 0
#define FORWARD 1
#define BACKWARD 2
#define RIGHT 3

//BOT definitions are used to set direction based on how the motors are
//plugged into the H-bridges and oriented.
#define BOT_LEFT 0x0A
#define BOT_FORWARD 0x0F
#define BOT_RIGHT 0x05
#define BOT_BACK 0x00

struct motorControl_t
{
	int speed_ticks			:16;
	
	int target_speed_ticks	:16;
	
	//used to control direction of motors 1 is forward 0 is backwards
	//not currently used but may be implemented later
	int LF_direction		:1;
	int RF_direction		:1;
	int LR_direction		:1;
	int RR_direction		:1;
	
	int direction			:2;
	
	int ramp_semaphore		:1;
	
};

void initialize_motorControl();
void setup_E0_motorControl();
void setup_E1_motorRamp();
void set_direction(uint8_t direction);
void set_speed_with_ramp(uint16_t desired_speed);
void set_speed_no_ramp(uint16_t desired_speed);
void disable_all_CCx_E0();
void enable_all_CCx_E0();
void turn_off_all_motors();
void turn_on_all_motors(uint16_t desiredSpeed);




#endif /* MOTOR_CONTROL_H_ */