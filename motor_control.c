/*
 * motor_control.c
 *
 * Created: 11/10/2015 4:38:57 PM
 *  Author: Clint
 *
 *	This file includes the functions that are used to control the speed and direction for the motors
 */ 
#include "motor_control.h"
#include "gpio.h"
#include "semaphores.h"
#include <avr/io.h>
#include <avr/interrupt.h>

struct motorControl_t;
extern struct motorControl_t motorControl;

void initialize_motorControl()
{
	motorControl.speed_ticks = 0;
	
	motorControl.target_speed_ticks = 0;
	
	//used to control direction of motors 1 is forward 0 is backwards
	motorControl.LF_direction = 0;
	motorControl.RF_direction = 0;
	motorControl.LR_direction = 0;
	motorControl.RR_direction = 0;
	
	motorControl.direction = FORWARD;
	
	//set initial direction to forward for all motors
	PORTD_OUT = 0x0f;

}

//Timer E0 is used to control the PWM for the motor speeds
void setup_E0_motorControl()
{
	//setup period for timer to 10000 ticks  (assuming 32MHz clock, this is 20ms with 64 prescaler)
	TCE0_PER = MAX_TICKS_MOTOR;

	//set prescaler for counter to 64 counts per 1 tick
	TCE0_CTRLA = 0x05;

	//enable CCA, CCB, CCC, CCD and use single slope waveform
	TCE0_CTRLB = 0xF3;

	//set CCA, CCB, CCC, CCD to 0 ticks
	TCE0_CCA = 0;
	TCE0_CCB = 0;
	TCE0_CCC = 0;
	TCE0_CCD = 0;

	//turn on access to CCA, CCB, CCC, CCD compare output value
	TCE0_CTRLC = 0x0F;

	//note that the signal for CCA is now on PE0, CCB on PE1, CCC on PE2, CCD on PE3
	
}

//Timer E1 is used when ramping up and down the motor speed to avoid drawing too much current and creating brown out.
void setup_E1_motorRamp()
{
	//setup period for timer to 20000 ticks  (assuming 32MHz clocks, this is 40ms with 64 prescale)
	TCE1_PER = MAX_TICKS_RAMP;

	//set prescaler for counter to 64 counts per 1 tick
	TCE1_CTRLA = 0x05;

	//set interrupt priority to low
	TCE1_INTCTRLA = 0x01;	
	
}

//unlock ramp semaphore when E1 overflows
ISR(TCE1_OVF_vect)
{
	motorControl.ramp_semaphore = 1;
}

void disable_all_CCx_E0()
{
	TCE0_CTRLB = 0x00;
}

void enable_all_CCx_E0()
{
	TCE0_CTRLB = 0xF3;
}

//used during debugging 
void turn_off_all_motors()
{
	while(motorControl.speed_ticks > MIN_SPEED_LIMIT_TICKS)
	{
		if(motorControl.ramp_semaphore)
		{
			motorControl.speed_ticks -= TICK_DELTA_MOTOR;
			TCE0_CCA = motorControl.speed_ticks;
			TCE0_CCB = motorControl.speed_ticks;
			TCE0_CCC = motorControl.speed_ticks;
			TCE0_CCD = motorControl.speed_ticks;
			motorControl.ramp_semaphore = 0;
		}
		
	}
	//check to make sure we didn't go lower than the desired speed
	if(motorControl.speed_ticks < MIN_SPEED_LIMIT_TICKS)
	{
		motorControl.speed_ticks = MIN_SPEED_LIMIT_TICKS;
		TCE0_CCA = motorControl.speed_ticks;
		TCE0_CCB = motorControl.speed_ticks;
		TCE0_CCC = motorControl.speed_ticks;
		TCE0_CCD = motorControl.speed_ticks;
	}
	
	disable_all_CCx_E0();
}

void turn_on_all_motors(uint16_t desiredSpeed)
{
	enable_all_CCx_E0();
	
	while(motorControl.speed_ticks < desiredSpeed)
	{
		if(motorControl.ramp_semaphore)
		{
			motorControl.speed_ticks += TICK_DELTA_MOTOR;
			TCE0_CCA = motorControl.speed_ticks;
			TCE0_CCB = motorControl.speed_ticks;
			TCE0_CCC = motorControl.speed_ticks;
			TCE0_CCD = motorControl.speed_ticks;
			motorControl.ramp_semaphore = 0;
		}
		
	}
	//check to make sure we didn't go past the limit
	if(motorControl.speed_ticks > desiredSpeed)
	{
		motorControl.speed_ticks = desiredSpeed;
		TCE0_CCA = motorControl.speed_ticks;
		TCE0_CCB = motorControl.speed_ticks;
		TCE0_CCC = motorControl.speed_ticks;
		TCE0_CCD = motorControl.speed_ticks;
	}
	
}


void set_direction(uint8_t direction)
{
	//if the bot is already traveling in the desired direction, we don't want to make it stop and start again
	if(direction == motorControl.direction){ return; }
	
	//save the current speed
	motorControl.target_speed_ticks = motorControl.speed_ticks;
	
	//ramp down to 0
	set_speed_with_ramp(0);
	
	//set the new direction
	switch(direction)
	{
		case(LEFT):	
			PORTD_OUT = BOT_LEFT;
			motorControl.direction = LEFT;
		
			break;
		case(FORWARD):
			PORTD_OUT = BOT_FORWARD;
			motorControl.direction = FORWARD;
		
			break;
		case(BACKWARD):
			PORTD_OUT = BOT_BACK;
			motorControl.direction = BACKWARD;
		
			break;
		case(RIGHT):
			PORTD_OUT = BOT_RIGHT;
			motorControl.direction = RIGHT;
		
			break;
			
	}
	
	//ramp back up to previous speed
	set_speed_with_ramp(motorControl.target_speed_ticks);
		
}


void set_speed_with_ramp(uint16_t desired_speed)
{
	//ramp down
	if(desired_speed < motorControl.speed_ticks)
	{
		while(motorControl.speed_ticks > desired_speed)
		{
			if(motorControl.ramp_semaphore)
			{
				motorControl.speed_ticks -= TICK_DELTA_MOTOR;
				TCE0_CCA = motorControl.speed_ticks;
				TCE0_CCB = motorControl.speed_ticks;
				TCE0_CCC = motorControl.speed_ticks;
				TCE0_CCD = motorControl.speed_ticks;
				motorControl.ramp_semaphore = 0;
			}
			
		}
		//check to make sure we aren't going to go negative
		if(motorControl.speed_ticks < TICK_DELTA_MOTOR)
		{
			motorControl.speed_ticks = 0;
			TCE0_CCA = motorControl.speed_ticks;
			TCE0_CCB = motorControl.speed_ticks;
			TCE0_CCC = motorControl.speed_ticks;
			TCE0_CCD = motorControl.speed_ticks;
		}
		
	}
	
	//ramp up but do 2 motors at a time, 1 from each H-bridge
	else
	{
		uint16_t speedHolder = motorControl.speed_ticks;
		
		//ramp up LF (CCA) and RR (CCC) first
		while(motorControl.speed_ticks < desired_speed)
		{
			//ramp up to 70% before starting motor to avoid drawing too much current at low speeds
			if(motorControl.speed_ticks < MIN_SPEED_LIMIT_TICKS)
			{
				motorControl.speed_ticks = MIN_SPEED_LIMIT_TICKS;
				TCE0_CCA = motorControl.speed_ticks;
				TCE0_CCC = motorControl.speed_ticks;
			}
			
			else if(motorControl.ramp_semaphore)
			{
				motorControl.speed_ticks += TICK_DELTA_MOTOR;
				TCE0_CCA = motorControl.speed_ticks;
				TCE0_CCC = motorControl.speed_ticks;
				motorControl.ramp_semaphore = 0;
			}
			
		}
		//check to make sure we didn't go past the limit
		if(motorControl.speed_ticks > desired_speed)
		{
			motorControl.speed_ticks = desired_speed;
			TCE0_CCA = motorControl.speed_ticks;
			TCE0_CCC = motorControl.speed_ticks;
		}		
		
		//reset motorControl
		motorControl.speed_ticks = speedHolder;
		
		//ramp up RF (CCD) and LR (CCB) second
		while(motorControl.speed_ticks < desired_speed)
		{
			//ramp up to 70% before starting motor to avoid drawing too much current
			if(motorControl.speed_ticks < MIN_SPEED_LIMIT_TICKS)
			{
				motorControl.speed_ticks = MIN_SPEED_LIMIT_TICKS;
				TCE0_CCB = motorControl.speed_ticks;
				TCE0_CCD = motorControl.speed_ticks;
			}
			
			else if(motorControl.ramp_semaphore)
			{
				motorControl.speed_ticks += TICK_DELTA_MOTOR;
				TCE0_CCB = motorControl.speed_ticks;
				TCE0_CCD = motorControl.speed_ticks;
				motorControl.ramp_semaphore = 0;
			}
			
		}
		
		//check to make sure we didn't go past the limit
		if(motorControl.speed_ticks > desired_speed)
		{
			motorControl.speed_ticks = desired_speed;
			TCE0_CCB = motorControl.speed_ticks;
			TCE0_CCD = motorControl.speed_ticks;
		}
		
	}
	
}


//This should only be used during debugging while observing PWM with O-Scope, it will cause brown out because
//The batteries/H-Bridge cannot supply enough current to start all motors at the same time. 
void set_speed_no_ramp(uint16_t desired_speed)
{
	TCE0_CCA = desired_speed;
	TCE0_CCB = desired_speed;
	TCE0_CCC = desired_speed;
	TCE0_CCD = desired_speed;
	
}

