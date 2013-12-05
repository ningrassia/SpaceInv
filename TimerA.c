#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include "lm4f120h5qr.h"

volatile bool TimerAAlert;

 /****************************************************************************
 * The TimerA Handler
 ****************************************************************************/
void TimerAIntHandler(void)
{ 
	// Acknowledge interrupt
	TIMER0_ICR_R = 0x10;
	
	// Report the interrupt by setting TimerAAlert
	TimerAAlert = true;
}

/****************************************************************************
 * Initialize the Timer A to a given count.
 * Turns on Interrupts
 ****************************************************************************/
void initializeTimerA(uint32_t count, bool enableInterrupts)
{
	//Enable the timer in the clock gating.
	volatile uint32_t delay;
	SYSCTL_RCGC1_R |= SYSCTL_RCGC1_TIMER0;
	delay = SYSCTL_RCGC1_R;
	
	//Configure the timer
	//Turn off timer0
	TIMER0_CTL_R &= ~TIMER_CTL_TAEN;

	//Clear the config register
	TIMER0_CFG_R = 0;
	
	//Set periodic mode
	TIMER0_TAMR_R |= TIMER_TAMR_TAMR_PERIOD;
	
	//Load start value
	TIMER0_TAILR_R = count;
	
	//Clear time-out!
	TIMER0_ICR_R = TIMER_ICR_TATOCINT;
	
	//Configure Interrupts
	if(enableInterrupts)
	{
		//Enable the interrupt
		TIMER0_IMR_R |= TIMER_IMR_TATOIM;
		
		//Enable NVIC interrupt ???
		NVIC_EN1_R |= 0x1<<3; //35-32 = 3, set bit 3 high to enable!
	}
	//Enable the timer!
	TIMER0_CTL_R |= TIMER_CTL_TAEN;
}
