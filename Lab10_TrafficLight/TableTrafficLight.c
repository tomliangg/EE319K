// ***** 0. Documentation Section *****
// TableTrafficLight.c for Lab 10
// Runs on LM4F120/TM4C123
// Index implementation of a Moore finite state machine to operate a traffic light.  
// Daniel Valvano, Jonathan Valvano
// January 15, 2016

// east/west red light connected to PB5
// east/west yellow light connected to PB4
// east/west green light connected to PB3
// north/south facing red light connected to PB2
// north/south facing yellow light connected to PB1
// north/south facing green light connected to PB0
// pedestrian detector connected to PE2 (1=pedestrian present)
// north/south car detector connected to PE1 (1=car present)
// east/west car detector connected to PE0 (1=car present)
// "walk" light connected to PF3 (built-in green LED)
// "don't walk" light connected to PF1 (built-in red LED)

// ***** 1. Pre-processor Directives Section *****
#include "TExaS.h"
#include "tm4c123gh6pm.h"

#define NVIC_ST_CTRL_R (*((volatile unsigned long *)0xE000E010))
#define NVIC_ST_RELOAD_R (*((volatile unsigned long *)0xE000E014))
#define NVIC_ST_CURRENT_R (*((volatile unsigned long *)0xE000E018))


#define SENSOR (*((volatile unsigned long *)0x4002401C))    //PE2-0
#define C_LIGHT (*((volatile unsigned long *)0x400050FC))  //car Light PB5-0
#define P_LIGHT (*((volatile unsigned long *)0x40025028))  //pedestrian Light PF3, PF1

#define goW 0
#define waitW 1
#define goS 2
#define waitS 3
#define goP 4
#define Flash_R1 5
#define Flash_O1 6
#define Flash_R2 7
#define Flash_O2 8

// Linked data structure
struct State {
  unsigned long PB_Out;  //output for Car Light
	unsigned long PF_Out;  //output for Pedestrian Light	
  unsigned long Time;  
  unsigned long Next[8];}; 
typedef const struct State STyp;

	
	//note: during virtual simulation, set wait time short bc the simulation is not real time
	//the virtual simulation runs 10~100 times slower than real time
STyp FSM[9]={
 {0x0C, 0x02,3000,{goW,goW,waitW,waitW,waitW,waitW,waitW,waitW}}, 
 {0x14, 0x02,500,{goS,goS,goS,goS,goP,goP,goS,goS}},
 {0x21, 0x02,3000,{goS,waitS,goS,waitS,waitS,waitS,waitS,waitS}},
 {0x22, 0x02,500,{goW,goW,goW,goW,goP,goP,goP,goP}},
 {0x24, 0x08,3000,{goP,Flash_R1,Flash_R1,Flash_R1,goP,Flash_R1,Flash_R1,Flash_R1}},
 {0x24, 0x02,500,{Flash_O1,Flash_O1,Flash_O1,Flash_O1,Flash_O1,Flash_O1,Flash_O1,Flash_O1}},
 {0x24, 0x00,500,{Flash_R2,Flash_R2,Flash_R2,Flash_R2,Flash_R2,Flash_R2,Flash_R2,Flash_R2}},
 {0x24, 0x02,500,{Flash_O2,Flash_O2,Flash_O2,Flash_O2,Flash_O2,Flash_O2,Flash_O2,Flash_O2}},
 {0x24, 0x00,500,{goW,goW,goS,goW,goW,goW,goS,goW}}};
// ***** 2. Global Declarations Section *****

// FUNCTION PROTOTYPES: Each subroutine defined
void DisableInterrupts(void); // Disable interrupts
void EnableInterrupts(void);  // Enable interrupts
void PortE_Init(void);

// ***** 3. Subroutines Section *****        this is part a
void PortB_Init(void){ volatile unsigned long delay;
  SYSCTL_RCGC2_R |= 0x00000002;     // 1) activate clock for Port B
  delay = SYSCTL_RCGC2_R;           // allow time for clock to start
  GPIO_PORTB_LOCK_R = 0x4C4F434B;   // 2) unlock GPIO Port B
  GPIO_PORTB_CR_R = 0x3F;           // allow changes to PB5-0
  // only PF0 needs to be unlocked, other bits can't be locked
  GPIO_PORTB_AMSEL_R = 0x00;        // 3) disable analog on PF
  GPIO_PORTB_PCTL_R = 0x00000000;   // 4) PCTL GPIO on PB5-0 clear bit
  GPIO_PORTB_DIR_R = 0x3F;          // 5) PB5-0 all output   0---> input;   1---> output
  GPIO_PORTB_AFSEL_R = 0x00;        // 6) disable alt funct on PB5-0
  //GPIO_PORTF_PUR_R = 0x11;          // enable pull-up on PF0 and PF4; this is not necessary
  GPIO_PORTB_DEN_R = 0x3F;          // 7) enable digital I/O on PB5-0
}

void PortE_Init(void){ volatile unsigned long delay;
  SYSCTL_RCGC2_R |= 0x00000010;     // 1) activate clock for Port E
  delay = SYSCTL_RCGC2_R;           // allow time for clock to start
  GPIO_PORTE_LOCK_R = 0x4C4F434B;   // 2) unlock GPIO Port E
  GPIO_PORTE_CR_R = 0x07;           // allow changes to PE2-0
  // only PF0 needs to be unlocked, other bits can't be locked
  GPIO_PORTE_AMSEL_R = 0x00;        // 3) disable analog on PE
  GPIO_PORTE_PCTL_R = 0x00000000;   // 4) PCTL GPIO on PE2-0 clear bit
  GPIO_PORTE_DIR_R = 0x00;          // 5) PE2-0 all input   0---> input;   1---> output
  GPIO_PORTE_AFSEL_R = 0x00;        // 6) disable alt funct on PE2-0
  //GPIO_PORTF_PUR_R = 0x11;          // enable pull-up on PF0 and PF4; this is not necessary
  GPIO_PORTE_DEN_R = 0x07;          // 7) enable digital I/O on PE2-0
}

void PortF_Init(void){ volatile unsigned long delay;
  SYSCTL_RCGC2_R |= 0x00000020;     // 1) activate clock for Port F
  delay = SYSCTL_RCGC2_R;           // allow time for clock to start
  GPIO_PORTF_LOCK_R = 0x4C4F434B;   // 2) unlock GPIO Port F
  GPIO_PORTF_CR_R = 0x0A;           // allow changes to PF3 (walk) and PF1 (don't walk)
  // only PF0 needs to be unlocked, other bits can't be locked
  GPIO_PORTF_AMSEL_R = 0x00;        // 3) disable analog on PF
  GPIO_PORTF_PCTL_R = 0x00000000;   // 4) PCTL GPIO on PF4-0 clear bit
  GPIO_PORTF_DIR_R = 0x0A;          // 5) PF3 and PF1 out   0---> input;   1---> output
  GPIO_PORTF_AFSEL_R = 0x00;        // 6) disable alt funct on PF7-0
  //GPIO_PORTF_PUR_R = 0x11;          // enable pull-up on PF0 and PF4; this is not necessary
  GPIO_PORTF_DEN_R = 0x0A;          // 7) enable digital I/O on PF3 and PF1
}

unsigned long S; // index to the current state 
unsigned long Input; 



void SysTick_Init(void){
	NVIC_ST_CTRL_R = 0; // disable SysTick during setup
	NVIC_ST_CTRL_R = 0x00000005; // enable SysTick with core clock
}
// The delay parameter is in units of the 80 MHz core clock. (12.5 ns)
void SysTick_Wait(unsigned long delay){
	NVIC_ST_RELOAD_R = delay-1; // number of counts to wait
	NVIC_ST_CURRENT_R = 0; // any value written to CURRENT clears
	while((NVIC_ST_CTRL_R&0x00010000)==0){ // wait for count flag
		}
}
// 80000*12.5ns equals 1ms
void SysTick_Wait1ms(unsigned long delay){
	unsigned long i;
	for(i=0; i<delay; i++){
		SysTick_Wait(80000); // wait 1ms
		}
}


int main(void){ 
  TExaS_Init(SW_PIN_PE210, LED_PIN_PB543210,ScopeOff); // activate grader and set system clock to 80 MHz
	PortB_Init();
	PortE_Init();
	PortF_Init();
	SysTick_Init();
  EnableInterrupts();
	S = goW;
  while(1){
		GPIO_PORTB_DATA_R = FSM[S].PB_Out; //set car lights
		GPIO_PORTF_DATA_R = FSM[S].PF_Out; //set pedestrian lights
		SysTick_Wait1ms(FSM[S].Time);  //wait time
		Input = SENSOR; //read sensor
		S = FSM[S].Next[Input];
  }
}


// Linked data structure

