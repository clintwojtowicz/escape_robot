/*
 * gpio.c
 *
 * Created: 11/12/2015 1:22:40 PM
 *  Author: Clint
 */ 

#include "gpio.h"
#include "led_definitions.h"
#include "motor_control.h"
#include "semaphores.h"
#include <avr/io.h>
#include <avr/interrupt.h>

extern struct semaphore_t semaphores;
extern struct motorControl_t motorControl;

void setup_gpio()
{
	//setup port B for input so infrared sensors can send their reaadings for ADC conversion
	PORTB_DIR = 0x00;
	
	//setup LEDs for debugging
	LED_PORT.DIR = 0xff;
	LED_PORT.OUT = 0x00;
	
	//port D is going to be used to control 
	//the motors' direction (hook up to phase pins on H-Bridge)
	PORTD_DIR = 0x0f;
	
	//turn on port E is used for PWM motor speed control, hook up to enable pins on H-bridge
	PORTE_DIR = 0xff;
	
}

void setup_btn_interrupt()
{
	//set port j for input from the buttons
	PORTJ_DIRSET = 0x00;
	
	//set interrupt for buttons 1 to 8
	PORTJ_INT0MASK = BUTTON_1 | BUTTON_2 | BUTTON_3 | BUTTON_4 | BUTTON_5 | BUTTON_6 | BUTTON_7 | BUTTON_8;
	
	//set buttons as a medium level interrupt
	PORTJ_INTCTRL = PMIC_MEDLVLEX_bm;
	
	//setup interrupt to fire on rising edge of button push
	PORTJ_PIN0CTRL = 0x01;
	PORTJ_PIN1CTRL = 0x01;
	PORTJ_PIN2CTRL = 0x01;
	PORTJ_PIN3CTRL = 0x01;
	PORTJ_PIN4CTRL = 0x01;
	PORTJ_PIN5CTRL = 0x01;
	PORTJ_PIN6CTRL = 0x01;
	PORTJ_PIN7CTRL = 0x01;
	
}

//interrupt for handling button presses
ISR(PORTJ_INT0_vect)
{
	LED_PORT.OUT ^= 0x80;	//toggle msb for debugging
	
	//use portj's input (i.e. which button is pressed) to figure out what to do
	switch(PORTJ_IN)
	{
		case (BUTTON_1):
		//set speed to 0
		motorControl.target_speed_ticks = 0;
		semaphores.change_speed = 1;
		
		break;
		
		case (BUTTON_2):
		//change speed to medium
		motorControl.target_speed_ticks = MOTOR_MEDIUM_TICKS;
		semaphores.change_speed = 1;
		
		break;
		
		case (BUTTON_3):
		//change speed to fast
		motorControl.target_speed_ticks = MOTOR_FAST_TICKS;
		semaphores.change_speed = 1;
		
		break;
		
		case (BUTTON_4):
		//change motor to left
		motorControl.direction = LEFT;
		semaphores.change_direction = 1;
		
		break;
		
		case(BUTTON_5):
		//change motor to forward
		motorControl.direction = FORWARD;
		semaphores.change_direction = 1;
		
		break;
		
		case(BUTTON_6):
		//change motor to backwards
		motorControl.direction = BACKWARD;
		semaphores.change_direction = 1;
		
		break;
		
		case(BUTTON_7):
		//change motor to right
		motorControl.direction = RIGHT;
		semaphores.change_direction = 1;
		
		break;
		
		case(BUTTON_8):
		
		
		break;
		default:
		//no valid button pressed do nothing
		break;
		
	}
	
	
}

