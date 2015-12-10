/*
 * semaphores.h
 *
 * Created: 11/10/2015 2:55:07 PM
 *  Author: Clint
 */ 

#ifndef SEMAPHORES_H_
#define SEMAPHORES_H_


struct semaphore_t
{
	int conversion_done		:1;
	
	// used to indicate when infrared measurements are complete
	int left_meas_done		:1;
	int right_meas_done		:1;
	int front_meas_done		:1;
	int back_meas_done		:1;
	
	int change_speed		:1;
	int change_direction	:1;
	
	int spin_complete		:1;
	int led_toggle			:1;
};

void initialize_semaphores();
void clear_meas_sems();



#endif /* SEMAPHORES_H_ */


