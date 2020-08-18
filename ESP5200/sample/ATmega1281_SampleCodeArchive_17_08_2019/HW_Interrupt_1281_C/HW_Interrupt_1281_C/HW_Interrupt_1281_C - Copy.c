//*****************************************
// Project 		Hardware Interrupt example (C)
// Target 		ATmega1281 on STK300
// Program		HW_Interrupt_1281_C.c
// Author		Richard Anthony
// Date			19th October 2013
//
// Function		Demonstrates Hardware interrupt

//				Displays a moving bar pattern on LEDs
//				The Hardware Interrupt 0 (PORT D bit 0) causes the moving bar pattern to reverse direction
//				I.e. Switch 0 causes an interrupt, and the interrupt handler reverses the bar-pattern direction

// Ports		Port B set for output (LEDs)
//				Port D bit 0 set for input (needed for the H/W interrupt to operate correctly)
//******************************************************************
#include <avr/io.h>
#include <avr/interrupt.h>
#include <util/delay.h>		// Needed so the _delay_ms() function can be used

#define DIRECTION_LEFT 0
#define DIRECTION_RIGHT 1

void InitialiseGeneral(void);
void Initialise_INT0();
void ENABLE_INT0(void);
void DISABLE_INT0(void);

unsigned char BarPattern;
unsigned char Direction;

int main(void)
{
	BarPattern = 1;
	Direction = DIRECTION_LEFT;
	Initialise_INT0();
	InitialiseGeneral();
	ENABLE_INT0();
	
    while(1)
    {
		PORTB = ~BarPattern;

		if(DIRECTION_LEFT == Direction)
		{
			BarPattern *= 2;
			if(0 == BarPattern)
			{	// When Bar pattern value wraps around (becomes 0), reset to 1
				BarPattern = 1;
			}
		}
		else
		{
			BarPattern /= 2;
			if(0 == BarPattern)
			{	// When Bar pattern reaches 0, reset to 128
				BarPattern = 128;
			}
		}
		
		_delay_ms(250);
	}
}

void InitialiseGeneral()
{
	DDRB = 0xFF;			// Configure PortB direction for Output
	PORTB = 0xFF;			// Set all LEDs initially off (inverted on the board, so '1' = off)	
		
	DDRD = 0x00;			// Port D Data Direction Register (Port D set for input)
	
	sei();					// Enable interrupts at global level, set Global Interrupt Enable (I) bit
}

void Initialise_INT0()
{
	// EICRA – External Interrupt Control Register A
	// Bits 7:0 – ISC31, ISC30 – ISC00, ISC00: External Interrupt 3 - 0 Sense Control Bits
	// 10  = The falling edge of INTn generates asynchronously an interrupt request
	EICRA = 0b00000010;		// Interrupt Sense (INT0) falling-edge triggered
	
	// EICRB – External Interrupt Control Register B
	// Bits 7:0 – ISC71, ISC70 - ISC41, ISC40: External Interrupt 7 - 4 Sense Control Bits
	// 10  = The falling edge of INTn generates asynchronously an interrupt request
	EICRB = 0b00000000;
	
	// EIMSK – External Interrupt Mask Register
	// Bits 7:0 – INT7:0: External Interrupt Request 7 - 0 Enable
	EIMSK = 0b00000000;		// Initially disabled, set bit 0 to Enable H/W Int 0
	
	// EIFR – External Interrupt Flag Register
	// Bits 7:0 – INTF7:0: External Interrupt Flags 7 - 0
	EIFR = 0b11111111;		// Clear all HW interrupt flags (in case a spurious interrupt has occurred during chip startup)
}

void ENABLE_INT0()
{
	EIMSK = 0b00000001;		// Enable H/W Int 0
}

void DISABLE_INT0()
{
	EIMSK = 0b00000000;		// Disable H/W Int 0
}

ISR(INT0_vect) // Interrupt Handler for H/W INT 0
{
	DISABLE_INT0();		// Disable INT0 to prevent key bounce causing multiple interrupt events
	if(DIRECTION_LEFT == Direction)		// Reverse direction of bar display
	{
		Direction = DIRECTION_RIGHT;
	}
	else
	{
		Direction = DIRECTION_LEFT;
	}	
	_delay_ms(80);		// Short delay to debounce the push-button
	ENABLE_INT0();		// Re-enable the interrupt
}

