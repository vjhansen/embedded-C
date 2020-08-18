/*******************************************
Project 		Binary Count on on-board LEDs
Target 			ATmega1281 on STK300
Program			BinaryCountOnLEDs_1281_C.c
Author			Richard Anthony
Date			14th September 2013

Fuse settings	System clock: Use the internal RC oscillator at 8.0MHz and CKDIV8 fuse programmed, resulting in 1.0MHz system clock.
				Fuse settings should be: CKSEL = "0010", SUT = "10", CKDIV8 = "0"

Function		Displays a binary count on the on-board LEDs
*******************************************/
#include <avr/io.h>
#include <util/delay.h>

void InitialiseGeneral();
unsigned char uCountvalue;	// Create the variable that will hold the count value

void InitialiseGeneral()
{
	// Configure Ports
	DDRB = 0xFF;	// Set port B direction OUTPUT (connected to the on-board LEDs)
	PORTB = 0xFF;	// Set all LEDs initially off (inverted on the board, so '1' = off)

	uCountvalue  = 0;	// Initialize the count value to 0
}

int main( void )
{
	InitialiseGeneral();

	while(1)
	{
		PORTB = ~uCountvalue;	// Write the data value to Port B (the ~ performs 1s compliment
								// because the switch values are inverted in the STK300 board hardware)
	
		uCountvalue++;			// Increment the count value
		_delay_ms(500);			// Add a delay so we can see the pattern change (try removing this delay)
	}
}