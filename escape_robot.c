/*
 * escape_robot.c
 *
 * Created: 11/9/2015 2:30:16 PM
 *  Author: Clint Wojtowicz and Matt Pianowski
 *
 *	This is designed for a 4 motor robot that will move in 4 different directions
 *	in order to escape detected threats. It will move away from the directed threat 
 *	and towards the area where the furthest threat is detected. 
 *
 *	The project is designed for use with the atxMega128A1
 *
 *	Library dependencies: libAVRX_Clocks.a which is used set system clock to 32MHz
 */ 


#include <avr/io.h>
#include <avr/interrupt.h>
#include <math.h>
#include <util/delay.h>
#include "semaphores.h"
#include "led_definitions.h"
#include "adc.h"
#include "sensors.h"
#include "direction_defs.h"
#include "motor_control.h"
#include "gpio.h"
#include "state_defs.h"


///////////////////  global variables
volatile unsigned long sClk, pClk;
volatile uint16_t threat_distance[4] = {0, 0, 0, 0};
volatile uint8_t closestThreat = 0;
volatile uint8_t furthestThreat = 0;
volatile struct semaphore_t semaphores;
volatile struct motorControl_t motorControl;
volatile struct infrResults_t infrResults;
volatile uint8_t state = 0;		//this is used to hold the current state of the robot


//Prototypes
void set_Clock_32MHz();
void initialize_threat_distances();
void determine_threat_order();
void move_away_from_threat();
uint8_t check_for_trapped();


void set_Clock_32MHz()
{
	SetSystemClock(CLK_SCLKSEL_RC32M_gc, CLK_PSADIV_1_gc, CLK_PSBCDIV_1_1_gc);
	GetSystemClocks(&sClk, &pClk);
}

void determine_threat_order()
{
	uint16_t closestThreat_meas = 0;
	uint16_t furthestThreat_meas = 0xFFFF;
	
	//check each direction to see which distance is closest or furthest
	//note that threat distance is the value returned by the ADC from infrared sensors, high = closer,  low = further away
	for(int i = LEFT; i <= RIGHT; i++)
	{
		//closest threat is used to decide what to move away from
		if (threat_distance[i] > closestThreat_meas )
		{
			closestThreat = (uint8_t)i;
			closestThreat_meas = threat_distance[i];
		}
		
		//furthest threat used to decide which direction to go
		if (threat_distance[i] < furthestThreat_meas )
		{
			furthestThreat = (uint8_t)i;
			furthestThreat_meas = threat_distance[i];
		}
			
	}
			
}

void move_away_from_threat()
{
	//check each direction to see which distance is furthest and move towards it
	for(int i = LEFT; i <= RIGHT; i++)
	{
		if (i == furthestThreat)
		{
			//make sure bot is moving away from something close, otherwise just let it sit and wait
			if(threat_distance[closestThreat] > MIN_INFRARED_THREAT)
			{
				if (motorControl.direction != furthestThreat)
				{
					set_direction(furthestThreat);
				}
				set_speed_with_ramp(MOTOR_FAST_TICKS);
				
			}
			else
			{
				//there is no threat within minimum threshold so just let the robot sit and wait
				set_speed_with_ramp(0);
			}
			
		}
		
	}

}

uint8_t check_for_trapped()
{
	//check each threat distance to see if they are all above max threshold
	for(int i = LEFT; i <= RIGHT; i++)
	{
		//if any of the measurements are less than the max threshold, it isn't trapped
		if (threat_distance[i] < TRAPPED_INFRARED) return 0;	
	}
	
	//if the function reaches this point, each threat measurement was above the max threshold and it is trapped
	return 1;	
	
}


int main(void)
{
	
	set_Clock_32MHz();
	initialize_semaphores();
	initialize_motorControl();
	initialize_threat_distances();
	reset_infSens();
	
	//clear interrupts
	cli();

	setup_timer_D1();			//D1 is used to control the timing for infrared sensor measurements
	setup_gpio();				//declares polarity for gpio ports
	setup_ADCB();				//sets up pins 0-3 for use with infrared sensors
	setup_E0_motorControl();	//E0 is used as PWM for controlling the motors
	setup_E1_motorRamp();		//E1 is the timer that is used for ramping up/down the pulse width in E0
	setup_btn_interrupt();		//sets up interrupts for buttons
	setup_C1_spinTimer();		//initialize timer used for controlling spin state
	setup_F1_LEDTimer();		//initialize timer used for toggling the LEDs

	
	//enable low, med, and high level interrupts
	PMIC_CTRL = PMIC_HILVLEN_bm |PMIC_MEDLVLEN_bm | PMIC_LOLVLEN_bm;
	
	//turn interrupts back on
	sei();
	
	//set motors to 0 ticks to start
	set_speed_with_ramp(0);
	
	//set state to escaping to start
	state = ESCAPING;

	while(1)
	{
		//escaping state
		while(state == ESCAPING)
		{
			//check to see if all measurements are done
			if(semaphores.left_meas_done && semaphores.back_meas_done && semaphores.front_meas_done && semaphores.right_meas_done)
			{
				//toggle lowest bit on LED's so that we can see the measurement status
				//LED_PORT.OUT ^= 0x01;
				
				//calculate the average distance measured by each infrared sensor
				set_infrSens_avg_to_threatDist();
				
				//check to see if the robot is trapped, i.e. all sides are above max threshold
				if(check_for_trapped())
				{
					state = TRAPPED;	//set state to trapped
					break;	//break out of while loop so that trapped state can be entered
				}
				
				determine_threat_order();
				
				move_away_from_threat();
				
				reset_infSens();
				
				clear_meas_sems();
				
				//show the closest threat on lowest nibble and furthest threat on upper nibble
				LED_PORT.OUT = (uint8_t)(closestThreat | (furthestThreat << 4));
				
			}	
			
		}	//end of escaping state while loop
		
		while(state == TESTING)
		{
			//check to see if change direction semaphore has been thrown by button press
			if(semaphores.change_direction)
			{
				set_direction(motorControl.direction);
				semaphores.change_direction = 0;
			}
			
			//check to see if change speed semaphore has been thrown by button press
			if(semaphores.change_speed)
			{
				set_speed_with_ramp(motorControl.target_speed_ticks);
				semaphores.change_speed = 0;
			}	
			
		}	//end of testing state
		
		while(state == TRAPPED)
		{		
			//set state to spinning
			state = SPINNING;
			
			
		}
		
		while(state == SPINNING)
		{
			//tell the robot to spin
			set_direction(SPIN_CC);
			set_speed_with_ramp(MOTOR_FAST_TICKS);
			
			//turn on LED timer for 100ms
			set_LEDTimer(50000);
			
			//turn on spinning timer so we can end the spin
			semaphores.spin_complete = 0;	//set to 0 so ISR can set it to 1
			set_spinTimer(SPIN_TICKS);
			
			//wait for the spin to finish, do LED light show while we wait
			while(!semaphores.spin_complete)
			{
				if(semaphores.led_toggle) 
				{
					next_spin_led();
					semaphores.led_toggle = 0;
				}
			}
			
			semaphores.spin_complete = 0;
			
			//turn off the spin timer
			set_spinTimer(0);
			//turn off LED_timer
			set_LEDTimer(0);
			//stop spinning
			set_speed_with_ramp(0);
				
			//after spin is done, return to escaping state
			//reset the sensors so they can start performing measurements
			reset_infSens();	
			clear_meas_sems();
			state = ESCAPING;
			
		}	//end of spinning state
		
		
	}
}