#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include "lm4f120h5qr.h"

volatile bool SysTickAlert;

 /****************************************************************************
 * The SysTick Handler
 ****************************************************************************/
void SYSTICKIntHandler(void)
{

  // Clear the SysTick Interrupt by reading the register
	uint32_t savedVal = NVIC_ST_CURRENT_R;
  
	// Report the interrupt by setting SysTickAlert
	SysTickAlert = true;

}

/****************************************************************************
 * Initialize the SysTick timer to a given count.
 * Turns on Interrupts
 ****************************************************************************/
void initializeSysTick(uint32_t count, bool enableInterrupts)
{
  uint32_t  savedVal = NVIC_ST_CTRL_R;
  NVIC_ST_CTRL_R     = 0;            // disable SysTick timer
  NVIC_ST_RELOAD_R   = count - 1;    // Set reload to count-1
  NVIC_ST_CURRENT_R = 0;            // clear the current count
  
  // Enable Interrupts
	if(enableInterrupts)
		NVIC_ST_CTRL_R = (NVIC_ST_CTRL_ENABLE | NVIC_ST_CTRL_CLK_SRC | NVIC_ST_CTRL_INTEN); //set the interrupt enable bit high. 
	// Or just turn on the SysTick.
	else
		NVIC_ST_CTRL_R = (NVIC_ST_CTRL_ENABLE | NVIC_ST_CTRL_CLK_SRC);
	

}
