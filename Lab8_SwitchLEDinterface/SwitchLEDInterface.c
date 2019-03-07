// ***** 0. Documentation Section *****
// SwitchLEDInterface.c for Lab 8
// Runs on LM4F120/TM4C123
// Use simple programming structures in C to toggle an LED
// while a button is pressed and turn the LED on when the
// button is released.  This lab requires external hardware
// to be wired to the LaunchPad using the prototyping board.
// January 15, 2016
//      Jon Valvano and Ramesh Yerraballi

// ***** 1. Pre-processor Directives Section *****
#include "TExaS.h"
#include "tm4c123gh6pm.h"

#define GPIO_PORTE_DATA_R       (*((volatile unsigned long *)0x400243FC))   //pg. 662
#define GPIO_PORTE_DIR_R        (*((volatile unsigned long *)0x40024400))   //pg. 663
#define GPIO_PORTE_AFSEL_R      (*((volatile unsigned long *)0x40024420))   //pg. 672
#define GPIO_PORTE_PUR_R        (*((volatile unsigned long *)0x40024510))		//pg. 678; we don't need pull-up actually in this lab
#define GPIO_PORTE_DEN_R        (*((volatile unsigned long *)0x4002451C))		//pg. 683
#define GPIO_PORTE_AMSEL_R      (*((volatile unsigned long *)0x40024528))		//pg. 687
#define GPIO_PORTE_PCTL_R       (*((volatile unsigned long *)0x4002452C))		//pg. 689
#define SYSCTL_RCGC2_R          (*((volatile unsigned long *)0x400FE108))		//pg. 464; Base = 0x400FE000, offset 0x108
#define SYSCTL_RCGC2_GPIOE      0x00000010  // port E Clock Gating Control; set bit 4 HIGH, see pg. 465
#define GPIO_PORTE_LOCK_R       (*((volatile unsigned long *)0x40024520))  	//pg. 684
#define GPIO_PORTE_CR_R         (*((volatile unsigned long *)0x40024524))		//pg. 685
	
// ***** 2. Global Declarations Section *****

// FUNCTION PROTOTYPES: Each subroutine defined
void PortE_Init(void);
void DisableInterrupts(void); // Disable interrupts
void EnableInterrupts(void);  // Enable interrupts
void Delay100ms(unsigned long); 

// ***** 3. Subroutines Section *****

// PE0, PB0, or PA2 connected to positive logic momentary switch using 10k ohm pull down resistor
// PE1, PB1, or PA3 connected to positive logic LED through 470 ohm current limiting resistor
// To avoid damaging your hardware, ensure that your circuits match the schematic
// shown in Lab8_artist.sch (PCB Artist schematic file) or 
// Lab8_artist.pdf (compatible with many various readers like Adobe Acrobat).
int main(void){ 
//**********************************************************************
// The following version tests input on PE0 and output on PE1
//**********************************************************************
  TExaS_Init(SW_PIN_PE0, LED_PIN_PE1, ScopeOn);  // activate grader and set system clock to 80 MHz
  PortE_Init();        // Call initialization of port PF1, PE0   
	
  EnableInterrupts();           // enable interrupts for the grader
  while(1){
    if ((GPIO_PORTE_DATA_R & 0x01)==0)
			GPIO_PORTE_DATA_R |= 0x02;  //the case that PE0 is released; note that PE0 is positive logic
		
		else
		{
			//this is the case that PE0 is pressed
			GPIO_PORTE_DATA_R^=0x02;
			Delay100ms(1);
		}
  }
  
}



void PortE_Init(void){ volatile unsigned long delay;
	SYSCTL_RCGC2_R |= 0x00000010;   	// 1) turn on the clock for port E
	delay = SYSCTL_RCGC2_R;           // delay   
	GPIO_PORTE_CR_R = 0x03;           // allow changes to PE1 and PE0
  //GPIO_PORTF_LOCK_R = 0x4C4F434B; // 2) no need to unlock for PE; unlocking is needed only for pins PC3-0,  7, PF0 on the LM4F
	GPIO_PORTE_AMSEL_R = 0x00;        // 3) disable analog mode on port E        
  GPIO_PORTE_PCTL_R = 0x00000000;   // 4) GPIO clear bit PCTL  
  GPIO_PORTE_DIR_R = 0x02;          // 5) PF0 input, PE1 output
  GPIO_PORTE_AFSEL_R = 0x00;        // 6) no alternate function (disbale alternate function on Port E)
  GPIO_PORTE_DEN_R = 0x03;          // 7) enable digital pins PE1 and PE0 
	//GPIO_PORTF_PUR_R = 0x10;        // No need pull up so I commented it out    
         
}



void Delay100ms(unsigned long time){
  unsigned long i;
  while(time > 0){
    i = 1333333;  // this number means 100ms
    while(i > 0){
      i = i - 1;
    }
    time = time - 1; // decrements every 100 ms
  }
}
