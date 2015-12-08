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




///////////////////  global variables
volatile unsigned long sClk, pClk;
volatile uint16_t threat_distance[4] = {0, 0, 0, 0};
volatile uint8_t closestThreat = 0;
volatile uint8_t furthestThreat = 0;
volatile struct semaphore_t semaphores;
volatile struct motorControl_t motorControl;
volatile struct infrResults_t infrResults;



//Prototypes
void set_Clock_32MHz();
void initialize_threat_distances();
void determine_threat_order();
void move_away_from_threat();


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
	for(int i = LEFT; i < RIGHT; i++)
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
	for(int i = LEFT; i < RIGHT; i++)
	{
		if (i == furthestThreat)
		{
			//make sure bot is moving away from something close, otherwise just let it sit and wait
			if(threat_distance[closestThreat] > MIN_INFRARED_THREAT)
			{
				set_direction(furthestThreat);
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

	
	//enable low, med, and high level interrupts
	PMIC_CTRL = PMIC_HILVLEN_bm |PMIC_MEDLVLEN_bm | PMIC_LOLVLEN_bm;
	
	//turn interrupts back on
	sei();
	
	//set motors to 0 ticks to start
	set_speed_with_ramp(0);

	while(1)
	{
		//check to see if all measurements are done 
		if(semaphores.left_meas_done && semaphores.back_meas_done && semaphores.front_meas_done && semaphores.right_meas_done)
		{
			//toggle lowest bit on LED's so that we can see the measurement status
			LED_PORT.OUT ^= 0x01;
			
			//calculate the average distance measured by each infrared sensor
			set_infrSens_avg_to_threatDist();
			
			determine_threat_order();
			
			move_away_from_threat();	
			
			reset_infSens();
			
			clear_meas_sems();
			
		}	
		
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
		
		
	}
}