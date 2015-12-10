/*
 * gpio.h
 *
 * Created: 11/12/2015 1:22:18 PM
 *  Author: Clint
 */ 


#ifndef GPIO_H_
#define GPIO_H_

#define BUTTON_1 0x01
#define BUTTON_2 0x02
#define BUTTON_3 0x04
#define BUTTON_4 0x08
#define BUTTON_5 0x10
#define BUTTON_6 0x20
#define BUTTON_7 0x40
#define BUTTON_8 0x80

void setup_gpio();
void setup_btn_interrupt();
void setup_F1_LEDTimer();
void next_spin_led();







#endif /* GPIO_H_ */