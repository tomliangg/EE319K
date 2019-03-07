// MeasurementOfDistance.c
// Runs on LM4F120/TM4C123
// Use SysTick interrupts to periodically initiate a software-
// triggered ADC conversion, convert the sample to a fixed-
// point decimal distance, and store the result in a mailbox.
// The foreground thread takes the result from the mailbox,
// converts the result to a string, and prints it to the
// Nokia5110 LCD.  The display is optional.
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

// Slide pot pin 3 connected to +3.3V
// Slide pot pin 2 connected to PE2(Ain1) and PD3
// Slide pot pin 1 connected to ground


#include "ADC.h"
#include "..//tm4c123gh6pm.h"
#include "Nokia5110.h"
#include "TExaS.h"

void EnableInterrupts(void);  // Enable interrupts

unsigned char String[10]; // null-terminated ASCII string
unsigned long Distance;   // units 0.001 cm
unsigned long ADCdata;    // 12-bit 0 to 4095 sample
unsigned long Flag;       // 1 means valid Distance, 0 means Distance is empty
void UART_ConvertDistance(unsigned long n);


//********Convert****************
// Convert a 12-bit binary ADC sample into a 32-bit unsigned
// fixed-point distance (resolution 0.001 cm).  Calibration
// data is gathered using known distances and reading the
// ADC value measured on PE1.  
// Overflow and dropout should be considered 
// Input: sample  12-bit ADC sample
// Output: 32-bit distance (resolution 0.001cm)
unsigned long Convert(unsigned long sample){
  return ((sample*2000)+4095-1)/4095; //we know that at full scale (when sampel is equal to 4095, the expected result is 2000)
														// since the full range is 2.0 cm, note that UART_ConvertDistance will make 2000 to 2.000 cm
	// simple round up method: ceil(A/B)=(A+B-1)/B. Have to do round up to get accurate result
}

// Initialize SysTick interrupts to trigger at 40 Hz, 25 ms
void SysTick_Init(unsigned long period){unsigned long volatile delay;
  SYSCTL_RCGC2_R |= 0x00000001; // activate port A
	delay = SYSCTL_RCGC2_R; //allow time to finish activation
	GPIO_PORTF_CR_R = 0x02; // allow changes to PF1
	GPIO_PORTF_AMSEL_R &= ~0x04; // no analog
	GPIO_PORTF_PCTL_R &= ~0x000000F0; // regular function
	GPIO_PORTF_DIR_R |= 0x02; // make PF1 out
	GPIO_PORTF_AFSEL_R &= ~0x02; // disable alt funct on PF3
	GPIO_PORTF_DEN_R |= 0x02; // enable digital I/O on PF3
	NVIC_ST_CTRL_R = 0; // disable SysTick during setup
	NVIC_ST_RELOAD_R = (80000000/period)-1; // reload value for 1/period s (assuming 80MHz), bascially set the period
	NVIC_ST_CURRENT_R = 0; // any write to current clears it
	NVIC_SYS_PRI3_R = NVIC_SYS_PRI3_R&0x40FFFFFF; // priority 2     NVIC_SYS_PRI3_R correspond to SysTick_Handler
	NVIC_ST_CTRL_R = 0x00000007; // enable with core clock and interrupts
	EnableInterrupts();
}
// executes every 25 ms, collects a sample, converts and stores in mailbox
void SysTick_Handler(void){ 
  GPIO_PORTF_DATA_R ^= 0x02;  //toggle PF1
	GPIO_PORTF_DATA_R ^= 0x02;  //toggle PF1 again
	ADCdata = ADC0_In(); //sample the ADC
	Distance = Convert(ADCdata); //convert the sample to distance
	UART_ConvertDistance(Distance);
	GPIO_PORTF_DATA_R ^= 0x02;  //toggle PF1 the third time
	//following is the optional: output the string
	Nokia5110_SetCursor(0, 0);
	Nokia5110_OutString(String); 
}

//-----------------------UART_ConvertDistance-----------------------
// Converts a 32-bit distance into an ASCII string
// Input: 32-bit number to be converted (resolution 0.001cm)
// Output: store the conversion in global variable String[10]
// Fixed format 1 digit, point, 3 digits, space, units, null termination
// Examples
//    4 to "0.004 cm"  
//   31 to "0.031 cm" 
//  102 to "0.102 cm" 
// 2210 to "2.210 cm"
//10000 to "*.*** cm"  any value larger than 9999 converted to "*.*** cm"
void UART_ConvertDistance(unsigned long n){
// as part of Lab 11 you implemented this function
	if (n < 1000) {
		String[0]='0';
		String[1]='.';
		
	if (n < 10) {
			String[2]='0';
			String[3]='0';
			String[4]=0x30 + n;
		}
		
	else if (n > 9 && n <100) {
			String[2]='0';
			String[3]=0x30 + n/10;
			String[4]=0x30 + n%10;
		}

	else if (n > 99) {
			String[2]=0x30 + n/100;
			String[3]=0x30 + (n/10)%10;
			String[4]=0x30 + n%10;
		}
		
	}
	
	else  {  //case that n > 999
		if (n<10000) {
			String[0]=0x30 + n/1000;
			String[1]='.';
			String[2]=0x30 + (n/100)%10;
			String[3]=0x30 + (n/10)%10;
			String[4]=0x30 + n%10;
		}
		
		else {
			String[0]='*';
			String[1]='.';
			String[2]='*';
			String[3]='*';
			String[4]='*';
		}
	}
	String[5]=' ';
	String[6]='c';
	String[7]='m';
	String[8]='\0';
}
//note: during debugging, only main is running. main1 and main2 are idle (not active)
// main1 is a simple main program allowing you to debug the ADC interface
int main1(void){ 
  TExaS_Init(ADC0_AIN1_PIN_PE2, UART0_Emulate_Nokia5110_NoScope);
  ADC0_Init();    // initialize ADC0, channel 1, sequencer 3
  EnableInterrupts();
  while(1){ 
    ADCdata = ADC0_In();
  }
}
// once the ADC is operational, you can use main2 to debug the convert to distance
int main2(void){ 
  TExaS_Init(ADC0_AIN1_PIN_PE2, UART0_Emulate_Nokia5110_NoScope);
  ADC0_Init();    // initialize ADC0, channel 1, sequencer 3
  Nokia5110_Init();             // initialize Nokia5110 LCD
  EnableInterrupts();
  while(1){ 
    ADCdata = ADC0_In();
    Nokia5110_SetCursor(0, 0);
    Distance = Convert(ADCdata);
    UART_ConvertDistance(Distance); // from Lab 11
    Nokia5110_OutString(String);    // output to Nokia5110 LCD (optional)
  }
}
// once the ADC and convert to distance functions are operational,
// you should use this main to build the final solution with interrupts and mailbox
int main(void){ 
  TExaS_Init(ADC0_AIN1_PIN_PE2, UART0_Emulate_Nokia5110_NoScope);
	ADC0_Init();// initialize ADC0, channel 1, sequencer 3
	Nokia5110_Init();// initialize Nokia5110 LCD (optional)
	SysTick_Init(40);// initialize SysTick for 40 Hz interrupts
	// initialize profiling on PF1 (optional), actually PF1 is done in SysTick_Init
  EnableInterrupts();
// print a welcome message  (optional)
  while(1){ 
// read mailbox
// output to Nokia5110 LCD (optional)
  }
}

