//*****************************************************************************
//
//*****************************************************************************
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include "lm4f120h5qr.h"
#include "board_config.h"
#include "team.h"
#include "UART.h"
#include "SPI.h"

/******************************************************************************
 * Defines
 *****************************************************************************/
#define PORTA                 0x40004000
#define PORTB                 0x40005000
#define PORTC                 0x40006000
#define PORTD                 0x40007000
#define PORTE                 0x40024000
#define PORTF                 0x40025000

#define RED				PIN_4
#define GREEN			PIN_5
#define BLUE			PIN_6

#define OUTPUT_ENABLE_B 	0xEF
#define ROW_EN						PIN_7
#define ENABLES_OFF				0x00

extern void PLL_Init(void);
extern void initPortC(void);
extern void EnableInterrupts(void);
extern void DisableInterrupts(void);
extern void initializeSysTick(uint32_t count, bool enableInterrupts);
extern void initializeTimerA(uint32_t count, bool enableInterrupts);

extern volatile bool SysTickAlert;
extern volatile bool TimerAAlert;

/******************************************************************************
 * Global Variables
 *****************************************************************************/
 GPIO_PORT *GpioPortA = (GPIO_PORT *)PORTA;
 GPIO_PORT *GpioPortB = (GPIO_PORT *)PORTB;
 GPIO_PORT *GpioPortC = (GPIO_PORT *)PORTC;
 GPIO_PORT *GpioPortD = (GPIO_PORT *)PORTD;
 GPIO_PORT *GpioPortE = (GPIO_PORT *)PORTE;
 GPIO_PORT *GpioPortF = (GPIO_PORT *)PORTF;
 
 /******************************************************************************
 * Functions
 *****************************************************************************/

//*****************************************************************************
// Initialize the GPIO port
//*****************************************************************************
bool  gpioPortInit( 
                    uint32_t baseAddress, 
                    uint32_t digitalEnableMask, 
                    uint32_t inputMask, 
                    uint32_t pullUpMask,
										uint32_t altFuncMask,
										uint32_t analogFuncMask,
										uint32_t portControl
                  )
{
	GPIO_PORT * ourPort = (GPIO_PORT *)baseAddress;
	volatile uint32_t delay;
	switch(baseAddress){
		case PORTA:
			SYSCTL_RCGCGPIO_R |= SYSCTL_RCGCGPIO_R0;
			break;
		
		case PORTB:
			SYSCTL_RCGCGPIO_R |= SYSCTL_RCGCGPIO_R1;
			break;
		
		case PORTD:
			SYSCTL_RCGCGPIO_R |= SYSCTL_RCGCGPIO_R3;
			break;
		
		case PORTE:
			SYSCTL_RCGCGPIO_R |= SYSCTL_RCGCGPIO_R4;
			break;
			
		case PORTF:
			SYSCTL_RCGCGPIO_R |= SYSCTL_RCGCGPIO_R5;
			break;
	}
	
	delay = SYSCTL_RCGCGPIO_R;

	ourPort->Direction &= ~inputMask; //0 is input for direction, 1 is input for inputMask
	ourPort->PullUpSelect |= pullUpMask;
	ourPort->AlternateFunctionSelect = altFuncMask;
	ourPort->DigitalEnable |= digitalEnableMask;
	ourPort->AnalogSelectMode |= analogFuncMask;
	ourPort->PortControl |= portControl;
	
  return true;
}

//Initialize UARTs 2 and 5 - need to update for interrupts!
void UART_Init(uint32_t baseAddress)
{
	uint32_t delay;
	UART_PERIPH * myUart;
	
	//get the struct
	myUart = (UART_PERIPH *) baseAddress;

	//enable clock gating
	switch(baseAddress)
	{
		case UART2:
			SYSCTL_RCGCUART_R |= SYSCTL_RCGCUART_R2;
			break;
		case UART5:
			SYSCTL_RCGCUART_R |= SYSCTL_RCGCUART_R5;
			break;
	}
	delay = SYSCTL_RCGCUART_R;
	
  // Set the baud rate to 115200
  myUart->IntegerBaudRateDiv = 43;           
  myUart->FracBaudRateDiv = 26;              

  // Configure the Line Control for 8-n-1
  myUart->LineControl = UART_LCRH_WLEN_8 | UART_LCRH_FEN;                  
  
  // Enable the UART - Need to enable both TX and RX
  myUart->UARTControl =  UART_CTL_RXE | UART_CTL_TXE | UART_CTL_UARTEN;
  
  // Wait until the UART is avaiable
	switch(baseAddress)
	{
			case UART2:
				while( !(SYSCTL_PRUART_R & SYSCTL_PRUART_R2 ))
					{}
				break;
			case UART5:
				while( !(SYSCTL_PRUART_R & SYSCTL_PRUART_R5 ))
					{}
				break;
	}
  
  delay = 500;
  while( delay != 0)
  {
    delay--;
  }
	
}

// Initialize the SPI interface
bool initializeSPI( uint32_t base, uint8_t phase, uint8_t polarity)
{
  uint32_t delay;
  SPI_PERIPH * myPeriph = (SPI_PERIPH *)base;

  // Turn on the Clock Gating Register
  switch (base) 
  {
    case SSI0 :
    {
				SYSCTL_RCGCSSI_R |= SYSCTL_RCGCSSI_R0;
				break;
    }
    default:
        return false;
  }

  delay = SYSCTL_RCGCSSI_R;

  // Disable the SSI interface
  myPeriph->SSICR1 = 0;

  // Enable Master Mode
  myPeriph->SSICR1 |= 0;

  // Assume that we hvae a 80MHz clock and want a 4MHz SPI clock
  // FSSIClk = FSysClk / (CPSDVSR * (1 + SCR))
  myPeriph->SSICPSR = 20;
  myPeriph->SSICR0  = 0;

  // Clear the phse and polarity bits
  myPeriph->SSICR0  &=  ~(SSI_CR0_SPH | SSI_CR0_SPO);

  if (phase == 1)
      myPeriph->SSICR0  = SSI_CR0_SPH;

  if (polarity ==1)
      myPeriph->SSICR0  |= SSI_CR0_SPO;

  // Freescale SPI Mode with 8-Bit data (See line 2226 of lm4f120h5qr.h)
  myPeriph->SSICR0  |= SSI_CR0_FRF_MOTO | SSI_CR0_DSS_8;

  //Enable SSI
  myPeriph->SSICR1 |= SSI_CR1_SSE;
	
  return true;
}

//Initialize the ADC
void initializeADC(void)
{
	uint32_t delay;
		// Enable ADC 
  SYSCTL_RCGCADC_R |= SYSCTL_RCGCADC_R0;
	delay = SYSCTL_RCGCADC_R;
	
	// Set up sequencer priorities
	ADC0_SSPRI_R = 0x0123;
	
	// Disable sequencer 3
	ADC0_ACTSS_R &= ~ADC_ACTSS_ASEN3;
	
	// Set trigger events - sequencer 3 is on a software trigger
	ADC0_EMUX_R &= ~ADC_EMUX_EM3_M;
	
	// Set up sample control bits same settings as book example
	ADC0_SSCTL3_R = ADC_SSCTL3_IE0 | ADC_SSCTL3_END0;

	// Enable sequencer 3
	ADC0_ACTSS_R |= ADC_ACTSS_ASEN3;
}

uint32_t GetADCval(uint32_t Channel)
{
  uint32_t result;

  ADC0_SSMUX3_R = Channel;      // Set the channel
  ADC0_ACTSS_R  |= ADC_ACTSS_ASEN3; // Enable SS3
  ADC0_PSSI_R = ADC_PSSI_SS3;     // Initiate SS3

  while(0 == (ADC0_RIS_R & ADC_RIS_INR3)); // Wait for END of conversion
  result = ADC0_SSFIFO3_R & 0x0FFF;     // Read the 12-bit ADC result
  ADC0_ISC_R = ADC_ISC_IN3;         // Acknowledge completion

  return result;
}

//Here we handle the switch stuff.
void sw2func(void)
{
}

void sw3func(void)
{
}

void sw4func(void)
{
}

void sw5func(void)
{
}

//Switch the mode!
void sw6func(void)
{
}

// Handle pushbuttons and their debouncing
// Check every 3 milliseconds, and if a button is low for 15 ms,
// return a button pushed event or something!
void pushButtonHandler(void)
{
	static uint8_t SW2_State = 0;
	static uint8_t SW3_State = 0;
	static uint8_t SW4_State = 0;
	static uint8_t SW5_State = 0;
	static uint8_t SW6_State = 0;
	
	if (GpioPortA->Data & SW2 == 0)
		SW2_State++;
	else
		SW2_State = 0;
	if (GpioPortA->Data & SW3 == 0)
		SW3_State++;
	else
		SW3_State = 0;
	if (GpioPortD->Data & SW4 == 0)
		SW4_State++;
	else
		SW4_State = 0;
	if (GpioPortD->Data & SW5 == 0)
		SW5_State++;
	else
		SW5_State = 0;
	if (GpioPortF->Data & SW6 == 0)
		SW6_State++;
	else
		SW6_State = 0;
	
	//handle one button at a time.
	if(SW2_State == 5)
	{
		sw2func();
	}
	else if(SW3_State == 5)
	{
		sw3func();
	}
	else if(SW4_State == 5)
	{
		sw4func();
	}
	else if(SW5_State == 5)
	{
		sw5func();
	}
	else if(SW6_State == 5)
	{
		sw6func();
	}
	
	
}

// Draw to the LEDs
void draw(int ourchar, int color, int row)
{
	//disable output! set it high
	GpioPortF->Data |= ~OUTPUT_ENABLE_B;
	
	//enable column select
	GpioPortC->Data = ROW_EN;
	//select row!
	switch(0)
	{
		case 0:
			GpioPortB->Data = ~PIN_0;
			break;
		case 1:
			GpioPortB->Data = ~PIN_1;
			break;
		case 2:
			GpioPortB->Data = ~PIN_2;
			break;
		case 3:
			GpioPortB->Data = ~PIN_3;
			break;
		case 4:
			GpioPortB->Data = ~PIN_4;
			break;
		case 5:
			GpioPortB->Data = ~PIN_5;
			break;
		case 6:
			GpioPortB->Data = ~PIN_6;
			break;
		case 7:
			GpioPortB->Data = ~PIN_7;
			break;
	}
	//disable column select
	GpioPortC->Data = ENABLES_OFF;
	
	//enable color
	GpioPortC->Data = color;
	//write line
	GpioPortB->Data = 0xF0; //needs to be from the character or whatever.
	//disable color
	GpioPortC->Data = ENABLES_OFF;
	//enable output
	GpioPortF->Data &= OUTPUT_ENABLE_B;

}

void clearDisp()
{
	//disable output! set it high
	GpioPortF->Data |= ~OUTPUT_ENABLE_B;
	
	//select a column
	GpioPortC->Data = ROW_EN;
	GpioPortB->Data = 0x0;
	GpioPortC->Data = ENABLES_OFF;
	//clear red
	GpioPortC->Data = RED;
	GpioPortB->Data = 0xFF;
	GpioPortC->Data = ENABLES_OFF;
	
	
	//clear green
	GpioPortC->Data = GREEN;
	GpioPortB->Data = 0xFF;
	GpioPortC->Data = ENABLES_OFF;

	//clear red
	GpioPortC->Data = BLUE;
	GpioPortB->Data = 0xFF;
	GpioPortC->Data = ENABLES_OFF;
	
	//enable output
	GpioPortF->Data &= OUTPUT_ENABLE_B;
}
 

//*****************************************************************************
//*****************************************************************************
int 
main(void)
{
  // Initialize the PLLs so the the main CPU frequency is 80MHz
  PLL_Init();
  
  //Configure Port C
  initPortC();
	
		//Configure the rest of the GPIO ports
	gpioPortInit(
								PORTB, 
								(PIN_0 | PIN_1 | PIN_2 | PIN_3 | PIN_4 | PIN_5 | PIN_6 | PIN_7),
								0x0,
								0x0,
								0x0,
								0x0,
								0x0
								);
	gpioPortInit(
								PORTF, 
								(SW6 | PIN_4),
								(SW6),
								(SW6),
								0x0,
								0x0,
								0x0
								);
	gpioPortInit(
								PORTD, 
								(SW4 | SW5),
								(SW4 | SW5 | PIN_6),
								(SW4 | SW5),
								(PIN_6 | PIN_7),
								0x0,
								0x0
								);
	gpioPortInit(
								PORTA,
								(SW2 | SW3 | PIN_2 | PIN_3 | PIN_4 | PIN_5),
								(SW2 | SW3 | PIN_2 | PIN_3 | PIN_4 | PIN_5),
								(SW2 | SW3),
								0x0,
								0x0,
								(GPIO_PCTL_PA5_SSI0TX | GPIO_PCTL_PA4_SSI0RX |
														GPIO_PCTL_PA3_SSI0FSS | GPIO_PCTL_PA2_SSI0CLK)
								);
	gpioPortInit(
								PORTE,
								(PIN_4 | PIN_5),
								(PIN_2 | PIN_3 | PIN_4),
								0x0,
								(PIN_2 | PIN_3 | PIN_4 | PIN_5),
								(PIN_2 | PIN_3),
								0x0
								);
								
	//Set up the SysTick Timer
	initializeSysTick(800000, true);
	
	//Set up TimerA - for debounce!
	initializeTimerA(2400000, true);
	
	//Set up both UARTs
	UART_Init(UART2);
	UART_Init(UART5);
	
	//main loop
	while(1)
	{
	}
  
}
