//*******************************************************************************
// Project 		Demonstration of programatically changing the System Clock DIVIDER of the Atmel microcontroller
// Target 		ATMEL ATmega1281 micro-controller on STK300 board
// Program		SystemClockSpeed_ProgrammaticChange_SingleLEDFlash_1281_C.c
// Author		Richard Anthony
// Date			16th September 2015

// Function			Shows how the system clock DIVIDER can be changed dynamically whilst the program is running
//					Increases, and then decreases the effective clock speed by changing the on-chip clock DIVIDER setting
//					Also demonstrates how to use the programmable timers
// 					Flashes a single LED at various rates, depending on the clock divider value
//					Shows the use of an Interrupt Handler

// I/O hardware	Uses on-board LEDs on Port B (output)
//*****************************************

#include <avr/io.h>
#include <avr/interrupt.h>
#include <avr/power.h>		// This header file provides functions to get / set the system clock prescaler

// Declare functions (these will be in a separate header file in larger programs)
void InitialiseGeneral();
void InitialiseTimer1();

// Declare global variables
clock_div_t SystemClockDIVIDER;

unsigned char iLED_value;	// An 'unsigned char' is an 8-bit numeric value, i.e. a single byte
unsigned char iPulseCount;	// Counts pulses so that the clock speed is changed after a given number of pulses at each speed setting

int main( void )
{
	InitialiseGeneral();
	iLED_value = 0x00;		// Initialise the variables
	iPulseCount = 0;
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

void InitialiseTimer1()		// Configure to generate an interrupt after a 10-Second interval
{	// The configuration assumes a 1MHz clock, but the system clock prescaler will be changed as the program is running,
	// so, without changing the configuration of this timer, the LED flashing rate will change because the entire system will be speeded up / slowed down
	TCCR1A = 0b00000000;	// Normal port operation (OC1A, OC1B, OC1C), Clear Timer on 'Compare Match' (CTC) waveform mode)
	TCCR1B = 0b00001101;	// CTC waveform mode, use prescaler 1024
	TCCR1C = 0b00000000;

	// For 1 MHz clock (with 1024 prescaler) to achieve a 10 second interval:
	// Need to count 10 million clock cycles (but already divided by 1024)
	// So actually need to count to (10000000 / 1024 =) 9766 decimal, = 2 * 4096 + 6 * 256 + 2 * 16 + 6 = 2626 Hex
	OCR1AH = 0x26; // Output Compare Registers (16 bit) OCR1BH and OCR1BL
	OCR1AL = 0x26;

	TCNT1H = 0b00000000;	// Timer/Counter count/value registers (16 bit) TCNT1H and TCNT1L
	TCNT1L = 0b00000000;
	TIMSK1 = 0b00000010;	// bit 1 OCIE1A		Use 'Output Compare A Match' Interrupt, i.e. generate an interrupt
							// when the timer reaches the set value (in the OCR1A register)
}

ISR(TIMER1_COMPA_vect) // TIMER1_CompareA_Handler (Interrupt Handler for Timer 1)
{	// Flip the value of the least significant bit of the 8-bit variable
	if(0 == iLED_value)
	{
		iLED_value = 1;
	}
	else
	{
		iLED_value = 0;
	}
	PORTB = ~iLED_value;
		
	iPulseCount++;
	if(2 == iPulseCount)
	{	
		SystemClockDIVIDER = clock_prescale_get();	// Gets and returns the clock divider register setting (return type is clock_div_t)
		// Returns clock_div_8 when the CKDIV8 fuse is set and the clock divider has not yet been changed in the program
		// So for example if the actual clock is 8MHz (as with the STK300 boards with their 8MHz quartz crystal) the effective speed is 8MHz divided by 8 = 1MHz

		switch(SystemClockDIVIDER)
		{	// Cycle through a sequence of different system clock divider values
			case clock_div_8:
				SystemClockDIVIDER = clock_div_4;
				break;
			case clock_div_4:
				SystemClockDIVIDER = clock_div_2;
				break;
			case clock_div_2:
				SystemClockDIVIDER = clock_div_1;
				break;
			case clock_div_1:
				SystemClockDIVIDER = clock_div_16;
				break;
			case clock_div_16:
				SystemClockDIVIDER = clock_div_8;
				break;
			default:
				SystemClockDIVIDER = clock_div_8;
				break;
		}
		clock_prescale_set(SystemClockDIVIDER); // Set the system clock divider to the new value
		iPulseCount = 0; // Reset the pulse count so the new setting is used for the same number of iterations
	}
}