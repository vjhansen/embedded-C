// *******************************************************************************
// Project 		Timer Demo2 Single-Timer LED Moving bar (Embedded C)
// Target 		ATMEL ATmega1281 micro-controller on STK300 board
// Program		TimerDemo2_SingleTimer_LED_MovingBar_1281_C.c
// Author		Richard Anthony
// Date			15th September 2013

// Function		Demonstrates how to use the programmable timers
// 				Produces a moving LED bar display
//					Also shows the use of an Interrupt Handler

//	Timer 'range'	Uses 8-bit Counter/Timer 0 (CT0) (there are 6 timers in total 0,1,2,3,4,5)
//					This timer can measure a time period of up to 255 clock pulses
//					(i.e.2 to the power of 8, minus 1)
//					(This is the biggest number we can load into the timer's register)

//					If the system clock is used directly
//						If the clock runs at 1Mhz, the maximum time period is
//						255 / 1000000 = 255 Microseconds
//						If the clock runs at 8Mhz, the maximum time period is
//						255 / 8000000 = 32 Microseconds

//	Prescaler		For this reason, clock prescalers are used.
//					These effectively slow down (divide) the input clock so that longer
//					periods can be measured
//					The prescaler options for timer 0 are:
//					1 (no prescaling), 8, 64, 256 or 1024

//					If the 8 prescaler is used
//						If the clock runs at 1Mhz, the maximum time period is
//						(255 * 8) / 1000000 = 2.04 Milliseconds
//						If the clock runs at 8Mhz, the maximum time period is
//						(255 * 8) / 8000000 = 255 Microseconds

//					If the 1024 prescaler is used
//						If the clock runs at 1Mhz, the maximum time period is
//						(255 * 1024) / 1000000 = 261 Milliseconds
//						If the clock runs at 8Mhz, the maximum time period is
//						(255 * 1024) / 8000000 = 32 Milliseconds

// 				To time a period slow enough to display a moving LED bar,
//				with a 1MHz system clock and an 8-bit timer it is necessary to use the 1024 prescaler

// I/O hardware	Uses on-board LEDs on Port B (output)
// *****************************************

#include <avr/io.h>
#include <avr/interrupt.h>

// Declare functions (these will be in a separate header file in larger programs)
void InitialiseGeneral();
void InitialiseTimer0();

// Declare global variables
unsigned char LED_value;	// An 'unsigned char' is an 8-bit numeric value, i.e. a single byte

int main( void )
{
	InitialiseGeneral();
	LED_value = 0x01;		// Set initial LED value (leftmost bar is on)

	InitialiseTimer0();
	
	while(1)        	// Loop
	{
		// In this example all the application logic is within the Timer1 interrupt handler
	}
}

void InitialiseGeneral()
{
	DDRB = 0xFF;			// Configure PortB direction for Output
	PORTB = 0xFF;			// Set all LEDs initially off (inverted on the board, so '1' = off)
	
	sei();
}

void InitialiseTimer0()		// Configure to generate an interrupt after a 1/4 Second interval
{
	TCCR0A = 0b00000000;	// Normal port operation (OC0A, OC0B), Normal waveform generation
	TCCR0B = 0b00000101;	// Normal waveform generation, Use 1024 prescaler
	// For 8 MHz clock (with 1024 prescaler) Overflow occurs after counting to 256
	// (but already divided by 1024)
	// So overflow occurs after 1024 * 256 / 8000000 = 0.033 seconds
	// So 8 bar movements (full range) will happen in 1/4 second

	// For 1 MHz clock (with 1024 prescaler) Overflow occurs after counting to 256
	// (but already divided by 1024)
	// So overflow occurs after 1024 * 256 / 1000000 = 0.26 seconds
	// So approximately 4 bar movements will happen in 1 second
	TCNT0 = 0b00000000;	// Timer/Counter count/value register

	TIMSK0 = 0b00000001;		// Use 'Overflow' Interrupt, i.e. generate an interrupt
								// when the timer reaches its maximum count value
}

ISR(TIMER0_OVF_vect) // TIMER0_Overflow_Handler (Interrupt Handler for Timer 0)
{	// Shift LED BAR left
	LED_value *= 2;		// Shift left by multiplying by 2
	if(0 == LED_value)	// Check if bar has now gone past the leftmost value
	{					// (if the value in the single-byte variable is shifted too far the resulting value will be 0
		LED_value = 1;	// Reset LED BAR position to right hand end
	}
	PORTB = ~LED_value;
}