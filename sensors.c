/*
 * sensors.c
 *
 * Created: 11/10/2015 4:44:50 PM
 *  Author: Clint
 *
 *	Also see adc.c for interrupts related to adc conversion completion
 *	
 *	The infrared sensors are set up to measure every 100ms and after 4 measurements, the 5th time through
 *	unlocks the measurements complete semaphores so that main can determine the threat direction and where
 *	the bot should travel. 
 *
 *  100ms			200ms			300ms			400ms			500ms
 *	measure			measure			measure			measure			unblock semaphores and calculate average, then start over.
 */ 
#include "sensors.h"
#include "led_definitions.h"
#include "direction_defs.h"
#include <avr/io.h>
#include <avr/interrupt.h>

//global variable declared in escape_robot.c
extern uint16_t threat_distance[4];

struct infrResults_t;
extern struct infrResults_t infrResults;	//global structure that is used to hold measurement results

//D1 is used to tell ADCB to do a conversion aka tells all the sensors to take a measurement
void setup_timer_D1()
{
	//setup period for timer (with 32MHz clock and prescale 64 (5), 50000 ticks is 100ms)
	TCD1_PER = 50000;
	
	//setup signal to generate when timer overflow occurs
	TCD1_INTCTRLA = PMIC_MEDLVLEX_bm;
	
	//set prescaler for counter to 64 ticks
	TCD1_CTRLA = 0x05;
	
}

ISR(TCD1_OVF_vect)
{
	//tell ADCB to perform conversions on all channels 0 to 3 (infrared sensors)
	ADCB_CTRLA |= 0x3C;

}

//D0 is used to control threat direction and level LED periods, used during debugging to give visualization for what
//the sensors are measuring
void setup_timer_D0()
{
	//setup period for timer to 60000 ticks  (assuming 32MHz clocks, this is 10ms)
	TCD0_PER = MAX_TICKS_THREAT_LED;
	
	//setup signal to generate when timer overflow occurs
	TCD0_INTCTRLA = PMIC_MEDLVLEX_bm;
	
	//set prescaler for counter to 4 counts per 1 tick
	TCD0_CTRLA = 0x02;
	
	//enable CCA, CCB, CCC, CCD
	TCD0_CTRLB = 0xF0;
	
	//enable interrupt for CCA, CCB, CCC, CCD to medium priority
	TCD0_INTCTRLB = 0xAA;
	
	//set CCA interrupt to occur when CCA reaches
	TCD0_CCA = MIN_TICK_DELTA;
	TCD0_CCB = MIN_TICK_DELTA;
	TCD0_CCC = MIN_TICK_DELTA;
	TCD0_CCD = MIN_TICK_DELTA;
	
}


void initialize_threat_distances()
{
	threat_distance[LEFT] = 0;
	threat_distance[RIGHT] = 0;
	threat_distance[FRONT] = 0;
	threat_distance[BACK] = 0;
}

//used for setting LED PWM period during debugging
double calculate_sigmoid(uint16_t sensor_value)
{
	double converted_value = (double)sensor_value;
	return 1/sqrt(1+pow((double)converted_value, 2));
	
}

//used for debugging sensors
void set_threatLevel_to_TCD0_CCx()
{
	TCD0_CCA = (uint16_t)(calculate_sigmoid(threat_distance[LEFT]) * MAX_TICKS_THREAT_LED * 500);
	TCD0_CCB = (uint16_t)(calculate_sigmoid(threat_distance[FRONT]) * MAX_TICKS_THREAT_LED * 500);
	TCD0_CCC = (uint16_t)(calculate_sigmoid(threat_distance[BACK]) * MAX_TICKS_THREAT_LED * 500);
	TCD0_CCD = (uint16_t)(calculate_sigmoid(threat_distance[RIGHT]) * MAX_TICKS_THREAT_LED *  500);
}

//Interrupts used for setting threat measurements from infrared sensors to lower nibble of LED port

//D0 used to control threat level LEDs (4 lsb)
ISR(TCD0_OVF_vect)
{
	//turn on all of the LEDs used on so that CCA, CCB, CCC, CCD can turn them off
	LED_PORT.OUT |= 0x0F;
}

ISR(TCD0_CCA_vect)
{
	//turn off left moving LED
	LED_PORT.OUT &= (~LED_THREAT_LEFT);
}

ISR(TCD0_CCB_vect)
{
	//turn off front moving LED
	LED_PORT.OUT &= (~LED_THREAT_FORWARD);
}

ISR(TCD0_CCC_vect)
{
	//turn off back moving LED
	LED_PORT.OUT &= (~LED_THREAT_BACKWARD);
}

ISR(TCD0_CCD_vect)
{
	//turn off right moving LED
	LED_PORT.OUT &= (~LED_THREAT_RIGHT);
	
	
}


//called by main to calculate the average threat distance measured by sensors every 500ms
void set_infrSens_avg_to_threatDist()
{
	threat_distance[LEFT] = calc_avg(LEFT);
	threat_distance[RIGHT] = calc_avg(RIGHT);
	threat_distance[FRONT] = calc_avg(FRONT);
	threat_distance[BACK] = calc_avg(BACK);
}


//calculates average of sensor measurements based on the direction passed
//threat measurements are stored in arrays in infResults structure
uint16_t calc_avg(uint8_t direction)
{
	int sum = 0;
	
	switch(direction)
	{
		case LEFT:
		
			for (int i = 0; i < NUM_INF_SENS_MEAS; i++)
			{
				sum += infrResults.left[i];
			}
		
			break;
		
		case FRONT:
		
			for (int i = 0; i < NUM_INF_SENS_MEAS; i++)
			{
				sum += infrResults.front[i];
			}
		
			break;
		
		case BACK:
		
			for (int i = 0; i < NUM_INF_SENS_MEAS; i++)
			{
				sum += infrResults.back[i];
			}
		
			break;
		
		case RIGHT:
		
			for (int i = 0; i < NUM_INF_SENS_MEAS; i++)
			{
				sum += infrResults.right[i];
			}
		
			break;	
		
	}	
	
	return sum / NUM_INF_SENS_MEAS;
	
}

//resets the measurement count, called by main after a direction has been determined every 500ms
void reset_infSens()
{
	infrResults.lCount = 0;
	infrResults.rCount = 0;
	infrResults.fCount = 0;
	infrResults.bCount = 0;
	
}


