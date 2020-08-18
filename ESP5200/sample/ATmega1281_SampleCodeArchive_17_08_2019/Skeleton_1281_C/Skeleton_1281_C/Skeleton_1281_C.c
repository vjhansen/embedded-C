/*******************************************
Project 		Skeleton code to be used as starting point for applications
Target 			ATmega1281 on STK300
Program			Skeleton_1281_C.c
Author			Richard Anthony
Date			14th September 2013

Fuse settings	System clock: Use the internal RC oscillator at 8.0MHz and CKDIV8 fuse programmed, resulting in 1.0MHz system clock.
				Fuse settings should be: CKSEL = "0010", SUT = "10", CKDIV8 = "0"

Function		Performs basic setup - no application logic included
*******************************************/
#include <avr/io.h>

void InitialiseGeneral();


void InitialiseGeneral()
{
	// Ports are configured by default to Port B output and Port D input - change as necessary
	DDRB = 0xFF;	// Set port B direction OUTPUT (connected to the on-board LEDs)
	PORTB = 0xFF;	// Set all LEDs initially off (inverted on the board, so '1' = off)

	DDRD = 0x00;	// Set port D direction INPUT (connected to the on-board SWITCHs)

	// Place additional one-off initialisation code here
}

int main( void )
{
	InitialiseGeneral();

	while(1)
	{
		// Place your main-loop code here
		
		// This loop will repeat continuously
	}
}