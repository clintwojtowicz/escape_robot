/*
 * semaphores.c
 *
 * Created: 11/10/2015 3:37:22 PM
 *  Author: Clint
 *
 *	Global semaphores used to unlock and lock certain actions 
 */ 


#include "semaphores.h"


struct semaphore_t;
extern struct semaphore_t semaphores;

void initialize_semaphores()
{
	semaphores.left_meas_done = 0;
	semaphores.right_meas_done = 0;
	semaphores.front_meas_done = 0;
	semaphores.back_meas_done = 0;
	
	semaphores.change_speed = 0;
	semaphores.change_direction = 0;
	
	semaphores.spin_complete = 0;
	semaphores.led_toggle = 0;
	
}

void clear_meas_sems()
{
	semaphores.left_meas_done = 0;
	semaphores.right_meas_done = 0;
	semaphores.front_meas_done = 0;
	semaphores.back_meas_done = 0;
	
	
}
