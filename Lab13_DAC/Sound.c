// Sound.c
// Runs on LM4F120 or TM4C123, 
// edX lab 13 
// Use the SysTick timer to request interrupts at a particular period.
// Daniel Valvano, Jonathan Valvano
// December 29, 2014
// This routine calls the 4-bit DAC

#include "Sound.h"
#include "DAC.h"
#include "..//tm4c123gh6pm.h"


unsigned char Index;
const unsigned char SineWave[16] = {4,5,6,7,8,9,10,11,12,11,10,9,8,7,6,5};
unsigned long period=0x05;
unsigned long SW_current;
unsigned long SW_old = 0;


// **************Sound_Init*********************
// Initialize Systick periodic interrupts
// Also calls DAC_Init() to initialize DAC
// Input: none
// Output: none
void Sound_Init(void){
	DAC_Init(); // Port B is DAC
	Index = 0;
	NVIC_ST_CTRL_R = 0; // disable SysTick during setup
  NVIC_ST_RELOAD_R = period-1;// reload value
	NVIC_ST_CURRENT_R = 0; // any write to current clears it
	NVIC_SYS_PRI3_R = (NVIC_SYS_PRI3_R&0x00FFFFFF)|0x20000000; //set priority to 1
	NVIC_ST_CTRL_R = 0x0007; // enable,core clock, and interrupts
}

// **************Sound_Tone*********************
// Change Systick periodic interrupts to start sound output
// Input: interrupt period
//           Units of period are 12.5ns
//           Maximum is 2^24-1
//           Minimum is determined by length of ISR
// Output: none
void Sound_Tone(unsigned long period_local){
// this routine sets the RELOAD and starts SysTick
	period = period_local;
}


// **************Sound_Off*********************
// stop outputing to DAC
// Output: none
void Sound_Off(void){
 // this routine stops the sound output
	GPIO_PORTB_DATA_R &= ~0x0F;
}


// Interrupt service routine
// Executed every 12.5ns*(period)
void SysTick_Handler(void){
	SW_current = GPIO_PORTE_DATA_R&0x0F;
	if (SW_current != SW_old) {
		if (GPIO_PORTE_DATA_R & 0x01) Sound_Tone(80000000/(16*523.251)); //set note C pitch
		if (GPIO_PORTE_DATA_R & 0x02) Sound_Tone(80000000/(16*587.330)); //set note D pitch
		if (GPIO_PORTE_DATA_R & 0x04) Sound_Tone(80000000/(16*659.255)); //set note E pitch
		if (GPIO_PORTE_DATA_R & 0x08) Sound_Tone(80000000/(16*783.991)); //set note G pitch
		Sound_Init();
	}
	Index = (Index+1)&0x0F; // 4,5,6,7,7,7,6,5,4,3,2,1,1,1,2,3... 
	DAC_Out(SineWave[Index]); // output one value each interrupt
	SW_old = SW_current;
}
