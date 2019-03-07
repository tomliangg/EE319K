// TuningFork.c Lab 12
// Runs on LM4F120/TM4C123
// Use SysTick interrupts to create a squarewave at 440Hz.  
// There is a positive logic switch connected to PA3, PB3, or PE3.
// There is an output on PA2, PB2, or PE2. The output is 
//   connected to headphones through a 1k resistor.
// The volume-limiting resistor can be any value from 680 to 2000 ohms
// The tone is initially off, when the switch goes from
// not touched to touched, the tone toggles on/off.
//                   |---------|               |---------|     
// Switch   ---------|         |---------------|         |------
//
//                    |-| |-| |-| |-| |-| |-| |-|
// Tone     ----------| |-| |-| |-| |-| |-| |-| |---------------
//
// Daniel Valvano, Jonathan Valvano
// January 15, 2016

/* This example accompanies the book
   "Embedded Systems: Introduction to ARM Cortex M Microcontrollers",
   ISBN: 978-1469998749, Jonathan Valvano, copyright (c) 2015

 Copyright 2016 by Jonathan W. Valvano, valvano@mail.utexas.edu
    You may use, edit, run or distribute this file
    as long as the above copyright notice remains
 THIS SOFTWARE IS PROVIDED "AS IS".  NO WARRANTIES, WHETHER EXPRESS, IMPLIED
 OR STATUTORY, INCLUDING, BUT NOT LIMITED TO, IMPLIED WARRANTIES OF
 MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE APPLY TO THIS SOFTWARE.
 VALVANO SHALL NOT, IN ANY CIRCUMSTANCES, BE LIABLE FOR SPECIAL, INCIDENTAL,
 OR CONSEQUENTIAL DAMAGES, FOR ANY REASON WHATSOEVER.
 For more information about my classes, my research, and my books, see
 http://users.ece.utexas.edu/~valvano/
 */


#include "TExaS.h"
#include "..//tm4c123gh6pm.h"
#define toggle 1
#define quiet 0

// basic functions defined at end of startup.s
void DisableInterrupts(void); // Disable interrupts
void EnableInterrupts(void);  // Enable interrupts
void WaitForInterrupt(void);  // low power mode
unsigned long state = quiet ; //set the inital state to be quiet
signed long SW_previous;  //It's very important to use signed number here!!! because if you use unsigned, and  
signed long SW_now;       // if you do (0x00 - 0x01) > 0, you will get true because of overflowing

// input from PA3, output from PA2, SysTick interrupts
void Sound_Init(void){ unsigned long volatile delay;
	SYSCTL_RCGC2_R |= 0x00000001; // activate port A
	delay = SYSCTL_RCGC2_R;
	GPIO_PORTA_CR_R = 0x0C; // allow changes to PA3-2
	GPIO_PORTA_AMSEL_R &= ~0x04; // no analog
	GPIO_PORTA_PCTL_R &= ~0x00F0FF00; // regular function
	GPIO_PORTA_DIR_R |= 0x04; // make PA2 out
	GPIO_PORTA_DIR_R &= ~0x08; //make PA3 in 
	GPIO_PORTA_DR8R_R |= 0x04; // can drive up to 8mA out
	GPIO_PORTA_AFSEL_R &= ~0x0C; // disable alt funct on PA3-2
	GPIO_PORTA_DEN_R |= 0x0C; // enable digital I/O on PA3-2
	NVIC_ST_CTRL_R = 0; // disable SysTick during setup
	NVIC_ST_RELOAD_R = (80000000/880)-1; // reload value for 1/880 s (assuming 80MHz)
	NVIC_ST_CURRENT_R = 0; // any write to current clears it
	NVIC_SYS_PRI3_R = NVIC_SYS_PRI3_R&0x40FFFFFF; // priority 2     NVIC_SYS_PRI3_R correspond to SysTick_Handler
	NVIC_ST_CTRL_R = 0x00000007; // enable with core clock and interrupts
	EnableInterrupts();
}

// called at 880 Hz
void SysTick_Handler(void){
	SW_now = GPIO_PORTA_DATA_R&0x08; //read the current switch (PA3) value;
	
	if (state == quiet && (SW_now - SW_previous) > 0 ) state = toggle; //For rising edge, change the state
	
	else if (state == toggle && (SW_now - SW_previous) > 0 ) state = quiet; //For rising edge, change the state
	
	if (state == quiet) GPIO_PORTA_DATA_R &= ~0x04; //For quiet state, make PA2 LOW

	else if (state == toggle) GPIO_PORTA_DATA_R ^= 0x04; //For toggle state, toggle PA2
	
	SW_previous = SW_now; //update the previous switch (PA3) value
}

int main(void){// activate grader and set system clock to 80 MHz
  TExaS_Init(SW_PIN_PA3, HEADPHONE_PIN_PA2,ScopeOn); 
  Sound_Init();
  EnableInterrupts();   // enable after all initialization are doneGPIO_PORTA_DEN_R ^= 0x04
	SW_previous = GPIO_PORTA_DATA_R&0x08; //read the previous PA3 value
  while(1){
    // main program is free to perform other tasks
    // do not use WaitForInterrupt() here, it may cause the TExaS to crash
  }
}
