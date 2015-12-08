/*
 * adc.c
 *
 * Created: 11/10/2015 4:35:32 PM
 *  Author: Clint
 */ 

#include "adc.h"
#include "semaphores.h"
#include "direction_defs.h"
#include "sensors.h"
#include <avr/io.h>
#include <avr/interrupt.h>

extern struct semaphore_t semaphores;
extern uint16_t threat_distance[4];
extern struct infrResults_t infrResults;

void setup_ADCB()
{
	//set ADCB for channels 0-3 and enable (0xC1)
	ADCB_CTRLA = 0xC1;
	
	//set voltage reference to 2.5v (AREFA)
	ADCB_REFCTRL = 0x20;
	
	//set resolution to 12 bits
	ADCB_CTRLB = ADC_RESOLUTION_12BIT_gc;
	
	//set interrupt for on complete and med level priority
	ADCB_CH0_INTCTRL = 0x02;
	ADCB_CH1_INTCTRL = 0x02;
	ADCB_CH2_INTCTRL = 0x02;
	ADCB_CH3_INTCTRL = 0x02;
	
	//set pre-scaler to divide by 4 (this was 512 for previous exp but 4 provides sufficient time and accuracy)
	ADCB_PRESCALER = ADC_PRESCALER_DIV4_gc;
	
	//enable pins 0 to 3 for ADC conversion, set mux channels to those pins
	ADCB.CH0.MUXCTRL = ADC_CH_MUXPOS_PIN0_gc;
	ADCB.CH1.MUXCTRL = ADC_CH_MUXPOS_PIN1_gc;
	ADCB.CH2.MUXCTRL = ADC_CH_MUXPOS_PIN2_gc;
	ADCB.CH3.MUXCTRL = ADC_CH_MUXPOS_PIN3_gc;
	
	//set the input mode
	ADCB.CH0.CTRL = ADC_CH_INPUTMODE_SINGLEENDED_gc | ADC_CH_GAIN_1X_gc;
	ADCB.CH1.CTRL = ADC_CH_INPUTMODE_SINGLEENDED_gc | ADC_CH_GAIN_1X_gc;
	ADCB.CH2.CTRL = ADC_CH_INPUTMODE_SINGLEENDED_gc | ADC_CH_GAIN_1X_gc;
	ADCB.CH3.CTRL = ADC_CH_INPUTMODE_SINGLEENDED_gc | ADC_CH_GAIN_1X_gc;

	//results are contained in: ADCA.CHx.RES
	
}

//////////	Interrupts for ADC conversion completion, see sensors.c for a timing diagram

ISR(ADCB_CH0_vect)
{
	if (infrResults.lCount < NUM_INF_SENS_MEAS)
	{
		infrResults.left[infrResults.lCount] = ADCB_CH0_RES;
		infrResults.lCount++;
	}
	else
	{
		semaphores.left_meas_done = 1;
	}
	
	
}

ISR(ADCB_CH1_vect)
{
	if (infrResults.fCount < NUM_INF_SENS_MEAS)
	{
		infrResults.front[infrResults.fCount] = ADCB_CH1_RES;
		infrResults.fCount++;
	}
	else
	{
		semaphores.front_meas_done = 1;
	}
	
}

ISR(ADCB_CH2_vect)
{
	
	if (infrResults.bCount < NUM_INF_SENS_MEAS)
	{
		infrResults.back[infrResults.bCount] = ADCB_CH2_RES;
		infrResults.bCount++;
	}
	else
	{
		semaphores.back_meas_done = 1;
	}
	
}

ISR(ADCB_CH3_vect)
{
	if (infrResults.rCount < NUM_INF_SENS_MEAS)
	{
		infrResults.right[infrResults.rCount] = ADCB_CH3_RES;
		infrResults.rCount++;
	}
	else
	{
		semaphores.right_meas_done = 1;
	}
	
}