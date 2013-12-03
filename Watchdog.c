#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include "lm4f120h5qr.h"

/******************************************************************************
 * Defines
 *****************************************************************************/
 #define WD0_EN          0x1
 #define TRIGGER_SYS_RST 0x2

void watchdogInit(uint32_t mSec)
{
	// Enable Watchdog 0 clock gating
	SYSCTL_RCGCWD_R |= WD0_EN;
	
	// Load watchdog with desired timer load value
	WATCHDOG0_LOAD_R = mSec;
	
	// Enable watchdog trigger system restart
	WATCHDOG0_CTL_R |= TRIGGER_SYS_RST;
}
