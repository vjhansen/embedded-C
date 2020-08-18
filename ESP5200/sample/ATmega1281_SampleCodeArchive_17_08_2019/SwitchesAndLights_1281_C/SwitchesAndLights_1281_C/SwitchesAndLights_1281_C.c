/*******************************************
Project 		Switches and Lights
Target 			ATmega1281 on STK300
Program			SwitchesAndLights_1281_C.c
Author			Richard Anthony
Date			14th September 2013

Fuse settings	System clock: Use the internal RC oscillator at 8.0MHz and CKDIV8 fuse programmed, resulting in 1.0MHz system clock.
				Fuse settings should be: CKSEL = "0010", SUT = "10", CKDIV8 = "0"

Function		Mirrors on-board switches onto on-board lights (LEDs)
*******************************************/
#include <avr/io.h>

void InitialiseGeneral();
unsigned char cDataPattern;	// Data value read from the switches, and then displayed on the LEDs

void InitialiseGeneral()
{
	// Configure Ports
	DDRB = 0xFF;	// Set port B direction OUTPUT (connected to the on-board LEDs)
	PORTB = 0xFF;	// Set all LEDs initially off (inverted on the board, so '1' = off)

	DDRD = 0x00;	// Set port D direction INPUT (connected to the on-board SWITCHs)
}

int main( void )
{
	InitialiseGeneral();

	while(1)
	{
		cDataPattern = ~PIND;	// Read the data value from Port D (the ~ performs 1s compliment 
								// because the switch values are inverted in the STK300 board hardware)

		PORTB = ~cDataPattern;	// Write the data value to Port B (the ~ performs 1s compliment
								// because the switch values are inverted in the STK300 board hardware)
	}
}