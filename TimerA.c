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
	// Report the interrupt by setting SysTickAlert
	TimerAAlert = true;
}

/****************************************************************************
 * Initialize the Timer A to a given count.
 * Turns on Interrupts
 ****************************************************************************/
void initializeTimerA(uint32_t count, bool enableInterrupts)
{
	
}
