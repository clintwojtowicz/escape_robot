/*
 * sensors.h
 *
 * Created: 11/10/2015 4:44:35 PM
 *  Author: Clint
 */ 


#ifndef SENSORS_H_
#define SENSORS_H_

#include <avr/io.h>

#define MAX_TICKS_THREAT_LED 60000
#define MIN_TICK_DELTA 50
#define MIN_INFRARED_THREAT 400
#define NUM_INF_SENS_MEAS 4

//these are used to get around having to share channel 2 because channel 3 isn't working
#define B_SEL 0
#define R_SEL 1

void setup_timer_D1();
void setup_timer_D0();
void initialize_threat_distances();
void set_threatLevel_to_TCD0_CCx();
double calculate_sigmoid(uint16_t sensor_value);
void set_infrSens_avg_to_threatDist();
uint16_t calc_avg(uint8_t direction);
void reset_infSens();
void initialize_infSens();

struct infrResults_t
{
	uint16_t left[NUM_INF_SENS_MEAS];
	uint16_t right[NUM_INF_SENS_MEAS];	
	uint16_t front[NUM_INF_SENS_MEAS];	
	uint16_t back[NUM_INF_SENS_MEAS];
		
	uint8_t lCount;
	uint8_t rCount;
	uint8_t fCount;
	uint8_t bCount;
	
	//since CH3 conversions aren't working, we have to switch CH2 back and forth between the sensors to
	//get reading for all 4 sensors
	int pinSel			:1;
	
};


#endif /* SENSORS_H_ */