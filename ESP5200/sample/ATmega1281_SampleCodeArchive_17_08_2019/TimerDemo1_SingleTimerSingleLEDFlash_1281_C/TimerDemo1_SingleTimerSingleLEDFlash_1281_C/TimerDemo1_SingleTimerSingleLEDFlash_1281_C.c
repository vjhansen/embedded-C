//*******************************************************************************
// Project 		Timer Demo1 Single-Timer Single-LED-Flash (Embedded C)
// Target 		ATMEL ATmega1281 micro-controller on STK300 board
// Program		TimerDemo1_SingleTimerSingleLEDFlash_1281_C.c
// Author		Richard Anthony
// Date			15th September 2013

// Function			Demonstrates how to use the programmable timers
// 					Flashes a single LED at 1Hz (1 flash per second)
//					Also shows the use of an Interrupt Handler

//	Timer 'range'	Uses 16-bit Counter/Timer 1 (CT1) (there are 6 timers in total 0,1,2,3,4,5)
//					This timer can measure a time period of up to 65535 clock pulses 
//					(i.e. 2 to the power of 16, minus 1)
//					(This is the biggest number we can load into the timer's register)

//					If the system clock is used directly
//						If the clock runs at 1Mhz, the maximum time period is
//						65535 / 1000000 = 65.5 Milliseconds
//						If the clock runs at 8Mhz, the maximum time period is
//						65535 / 8000000 = 8.2 Milliseconds

//	Prescaler		For this reason, clock prescalers are used. 
//					These effectively slow down (divide) the input clock so that longer
//					periods can be measured
//					The prescaler options for timer 1 are:
//					1 (no prescaling), 8, 64, 256 or 1024

//					If the 8 prescaler is used
//						If the clock runs at 1Mhz, the maximum time period is
//						(65535 * 8) / 1000000 = 524 Milliseconds
//						If the clock runs at 8Mhz, the maximum time period is
//						(65535 * 8) / 8000000 = 65.5 Milliseconds

//					If the 1024 prescaler is used
//						If the clock runs at 1Mhz, the maximum time period is
//						(65535 * 1024) / 1000000 = 67 Seconds
//						If the clock runs at 8Mhz, the maximum time period is
//						(65535 * 1024) / 8000000 = 8.4 Seconds

// 					To time a period of 1 second, with either 1MHz or 8MHz system clock
//					it is necessary to use either the 256 or 1024 prescaler

// I/O hardware	Uses on-board LEDs on Port B (output)
//*****************************************

#include <avr/io.h>
#include <avr/interrupt.h>

// Declare functions (these will be in a separate header file in larger programs)
void InitialiseGeneral();
void InitialiseTimer1();


// Declare global variables
unsigned char LED_value;	// An 'unsigned char' is an 8-bit numeric value, i.e. a single byte

int main( void )
{
	InitialiseGeneral();
	LED_value = 0x00;		// Initialise the variable
	InitialiseTimer1();
	
    while(1)        	// Loop
    {
	}
}

void InitialiseGeneral()
{
	DDRB = 0xFF;			// Configure PortB direction for Output
	PORTB = 0xFF;			// Set all LEDs initially off (inverted on the board, so '1' = off)	
	DDRD = 0x00;			// Configure PortD direction for Input
	
	sei();					// Enable interrupts at global level, set Global Interrupt Enable (I) bit
}

void InitialiseTimer1()		// Configure to generate an interrupt after a 1-Second interval
{
	TCCR1A = 0b00000000;	// Normal port operation (OC1A, OC1B, OC1C), Clear Timer on 'Compare Match' (CTC) waveform mode)
	TCCR1B = 0b00001101;	// CTC waveform mode, use prescaler 1024
	TCCR1C = 0b00000000;
	
	// For 1 MHz clock (with 1024 prescaler) to achieve a 1 second interval:
	// Need to count 1 million clock cycles (but already divided by 1024)
	// So actually need to count to (1000000 / 1024 =) 976 decimal, = 3D0 Hex
	OCR1AH = 0x03; // Output Compare Registers (16 bit) OCR1BH and OCR1BL
	OCR1AL = 0xD0;

	TCNT1H = 0b00000000;	// Timer/Counter count/value registers (16 bit) TCNT1H and TCNT1L
	TCNT1L = 0b00000000;
	TIMSK1 = 0b00000010;	// bit 1 OCIE1A		Use 'Output Compare A Match' Interrupt, i.e. generate an interrupt
							// when the timer reaches the set value (in the OCR1A register)	
}

ISR(TIMER1_COMPA_vect) // TIMER1_CompareA_Handler (Interrupt Handler for Timer 1)
{	// Flip the value of the least significant bit of the 8-bit variable
	if(0 == LED_value)
	{
		LED_value = 1;
	}
	else
	{
		LED_value = 0;
	}
	PORTB = ~LED_value;
}