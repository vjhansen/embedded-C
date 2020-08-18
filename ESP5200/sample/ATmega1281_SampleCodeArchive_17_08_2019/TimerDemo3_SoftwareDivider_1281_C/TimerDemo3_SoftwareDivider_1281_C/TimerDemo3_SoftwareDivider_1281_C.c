// *******************************************************************************
// Project 		Timer Demo3 Extending the range of a timer using a software divider (Embedded C)
// Target 		ATMEL ATmega1281 micro-controller on STK300 board
// Program		TimerDemo3_SoftwareDivider_1281_C.c
// Author		Richard Anthony
// Date			15th September 2013

// Function		Demonstrates how to use the programmable timers
//					Implements a 'software divider' to extend the timer range
//					Provides multiple different time periods using only a single programmable timer


// 				Flashes a several LEDs at different frequencies
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
//					Software is then used to count multiples of the interrupt activations
//					and thus flash LEDs 0 and 7 at different frequencies

// I/O hardware		Uses on-board LEDs on Port B (output)
// *****************************************

#include <avr/io.h>
#include <avr/interrupt.h>

// Declare functions (these will be in a separate header file in larger programs)
void InitialiseGeneral();
void InitialiseTimer1();
void Toggle_LED0();
void Toggle_LED7();

// Declare global variables
unsigned char LED7_count;
unsigned char LED7_interval;

int main( void )
{
	InitialiseGeneral();
	LED7_count = 0;			// Initialise the count to 0
	LED7_interval = 5;		// Set the interval for LED7 blinking

	InitialiseTimer1();
	
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
{
	Toggle_LED0();		// Bit 0 is toggled every time the interrupt occurs
	
	// SoftwareDivider logic, counts a number of interrupts, before taking action
	LED7_count++;
	if(LED7_count == LED7_interval)		// Have we reached the required number of interrupts?
	{
		LED7_count = 0;					// Reset the software counter for this event
		Toggle_LED7();					// Yes - so change the LED value
	}

	// Reset the count value back to zero (needed when using 'normal mode')
	// Alternatively, if CTC mode is used the timer is automatically cleared
	TCNT1H = 0x00;
	TCNT1L = 0x00;
}

void Toggle_LED0()	// Swap the value of port B bit 0 (if 0, set to 1; if 1 set to 0)
{
	unsigned char temp;
	temp = PINB;			// (Can actually read the port value even when its set as output)
	if(temp & 0b00000001)	// Bitwise version of AND - Check if bit 0 is currently set '1'
	{
		temp &= 0b11111110;	// Currently set, so force it to '0' without changing other bits
	}
	else
	{
		temp |= 0b00000001;	// Currently cleared, so force it to '1' without changing other bits
	}
	PORTB = temp;
}

void Toggle_LED7()	// Swap the value of port B bit 7 (if 0, set to 1; if 1 set to 0)
{
	unsigned char temp;
	temp = PINB;			// (Can actually read the port value even when its set as output)
	if(temp & 0b10000000)	// Check if bit 7 is currently set '1'
	{
		temp &= 0b01111111;	// Currently set, so force it to '0' without changing other bits
	}
	else
	{
		temp |= 0b10000000;	// Currently cleared, so force it to '1' without changing other bits
	}
	PORTB = temp;
}
