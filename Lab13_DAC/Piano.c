// Piano.c
// Runs on LM4F120 or TM4C123, 
// edX lab 13 
// There are four keys in the piano
// Daniel Valvano
// December 29, 2014

// Port E bits 3-0 have 4 piano keys

#include "Piano.h"
#include "..//tm4c123gh6pm.h"


// **************Piano_Init*********************
// Initialize piano key inputs
// Input: none
// Output: none
void Piano_Init(void){unsigned long volatile delay;
	SYSCTL_RCGC2_R |= 0x10; // activate port E  
	delay = SYSCTL_RCGC2_R; // allow time to finish activating
	GPIO_PORTE_AMSEL_R &= ~0x0F; // no analog
	GPIO_PORTE_PCTL_R &= ~0x0000FFFF; // regular GPIO function
	GPIO_PORTE_DIR_R &= ~0x0F; // make PE3-0 in
	GPIO_PORTE_AFSEL_R &= ~0x0F; // disable alt funct on PE3-0
  GPIO_PORTE_DEN_R |= 0x0F; // enable digital I/O on PE3-0
}
// **************Piano_In*********************
// Input from piano key inputs
// Input: none 
// Output: 0 to 15 depending on keys
// 0x01 is key 0 pressed, 0x02 is key 1 pressed,
// 0x04 is key 2 pressed, 0x08 is key 3 pressed
unsigned long Piano_In(void){
  if (GPIO_PORTE_DATA_R & 0x01) GPIO_PORTB_DATA_R |= 0x01;
	else if ((GPIO_PORTE_DATA_R & 0x01) != 1) GPIO_PORTB_DATA_R &= 0x01;

  if (GPIO_PORTE_DATA_R & 0x02) GPIO_PORTB_DATA_R |= 0x02;
	else if ((GPIO_PORTE_DATA_R & 0x02) != 1) GPIO_PORTB_DATA_R &= 0x02;
	
	if (GPIO_PORTE_DATA_R & 0x04) GPIO_PORTB_DATA_R |= 0x04;
	else if ((GPIO_PORTE_DATA_R & 0x04) != 1) GPIO_PORTB_DATA_R &= 0x04;
	
  if (GPIO_PORTE_DATA_R & 0x08) GPIO_PORTB_DATA_R |= 0x08;
	else if ((GPIO_PORTE_DATA_R & 0x08) != 1) GPIO_PORTB_DATA_R &= 0x08;
	
  return GPIO_PORTB_DATA_R; //return the output corresponding to the input piano key 
}
