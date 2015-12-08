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
	
};


#endif /* SENSORS_H_ */