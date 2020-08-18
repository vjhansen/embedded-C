/*******************************************
** Project 		Seven Segment Display Demonstration
** Target 		ATmega1281 on STK300
** Program		SevenSegmentDisplay_1281_C.c
** Author		Richard Anthony
** Date			10th October 2013 (ATmega8535 version 2nd September 2008)
**
** Function		Counts through 2 Hex digits using the seven segment display as output
**
** I/O hardware	Uses delay-loop instead of Timer (for demonstration before timers  are covered)
**					Uses Seven Segment Display on Port B

**					Uses on-board switches to select interlace rate between the two characters
**					This is to allow slowing down the interlace to see the alternation of the character display
**					In a final application a high interlace rate would be used so the user does not detect any flicker
**					Switch 7 = 1 HZ interlace
**					Switch 6 = 1.8 HZ interlace
**					Switch 5 = 3 HZ interlace
**					Switch 4 = 5 HZ interlace
**					Switch 3 = 10 HZ interlace
**					Switch 2 = 20 HZ interlace
**					Switch 1 = 30 HZ interlace
**					Switch 0 = 40 HZ interlace
*******************************************/

#include <avr/io.h>

#define NumCharPatterns 16

void InitialiseGeneral();
void SetSevenSegmentCharacterPatternsInArray();
void DisplayHexValueToSevenSegmentDisplay();

unsigned char CharacterArray[NumCharPatterns]; // To hold display patterns for characters '0' - '9' and 'A' - 'F'
unsigned char uDisplayFlag;
unsigned char uCount; // The number to display 00 - FF
unsigned char uCount_LSD; // The lower digit of number to display
unsigned char uCount_MSD; // The upper digit of number to display
int iInterval;
int iIntervalCount;

void InitialiseGeneral()
{
	DDRB = 0xFF;	// Set port B direction OUTPUT (connected to the Seven Segment display)
	PORTB = 0x00;	// Set all seven-segment LEDs initially off

	DDRD = 0x00;	// Set port D direction INPUT (connected to the on-board SWITCHs)
}

int main( void )
{
	unsigned char uSWITCH_VALUE = 0x01;
	unsigned char uDELAY_TIME = 0xFF;	// Initialise to slowest speed
	uCount  = 0;
	
	iInterval = 500 / uDELAY_TIME;
	iIntervalCount = iInterval;
	uDisplayFlag = 0x00;	// Initially display Least Significant Digit

	InitialiseGeneral();
	SetSevenSegmentCharacterPatternsInArray();

	while(1)
	{
		uCount_LSD = uCount % 16;
		uCount_MSD = uCount / 16;
		DisplayHexValueToSevenSegmentDisplay();
		uDisplayFlag = ~uDisplayFlag;	// Controls the interlacing between LSD and MSB (Alternate Character)

		uSWITCH_VALUE = ~PIND;	// Read control value from on-board switches
								// 1's complement because on-board switch values are inverted

		// Configure interlace and character update interval parameter based on switch selection
		switch(uSWITCH_VALUE)
		{
			case 0b00000001:	// Switch 0 was pressed
				uDELAY_TIME = 0x02;	// Set fast speed (Cannot see flicker)
				break;
			
			case 0b00000010:	// Switch 1 was pressed
				uDELAY_TIME = 0x08;	// Set speed (Flicker is visible, very fast)
				break;

			case 0b00000100:	// Switch 2 was pressed
				uDELAY_TIME = 0x10; 	// Set speed (Flicker clearly visible)
				break;
		
			case 0b00001000:	// Switch 3 was pressed
				uDELAY_TIME = 0x20; 	// Set speed
				break;

			case 0b00010000:	// Switch 4 was pressed
				uDELAY_TIME = 0x40; 	// Set speed
				break;
		
			case 0b00100000:	// Switch 5 was pressed
				uDELAY_TIME = 0x70; 	// Set slow speed
				break;

			case 0b01000000:	// Switch 6 was pressed
				uDELAY_TIME = 0xB0; 	// Set speed
				break;
		
			case 0b10000000:	// Switch 7 was pressed
				uDELAY_TIME = 0xFF; 	// Set slow speed
				break;
		}	
		
		iInterval = 500 / uDELAY_TIME;
		
		for(int iDelay = 0; iDelay < (uDELAY_TIME * 40) ; iDelay++)
		{
				int kDelay = 9;	
		}
	}	
}

void DisplayHexValueToSevenSegmentDisplay()
{
	unsigned char uDisplayPattern;
	
	if(0x00 == uDisplayFlag)		// 0x00 means display LSD, 0xFF means display MSD
	{	// ShowLSD
		uDisplayPattern = CharacterArray[uCount_LSD];
		uDisplayPattern |= 0b10000000;	// Force MSB to '1'
		PORTB = uDisplayPattern;			// Display low digit
	}
	else
	{	// ShowMSD
		uDisplayPattern = CharacterArray[uCount_MSD];
		uDisplayPattern &= 0b01111111;	// Force MSB to '0'
		PORTB = uDisplayPattern;			// Display low digit
	}
	
	if(0 == --iIntervalCount)
	{	// Get next value to display (happens at a much slower rate than the interlace)
		uCount++;
		iIntervalCount = iInterval;
	}
}

void SetSevenSegmentCharacterPatternsInArray()
{
	CharacterArray[0] = 0b00111111;		// character code for '0'
	CharacterArray[1] = 0b00000110;		// character code for '1'
	CharacterArray[2] = 0b01011011;		// character code for '2'
	CharacterArray[3] = 0b01001111;		// character code for '3'
	CharacterArray[4] = 0b01100110;		// character code for '4'
	CharacterArray[5] = 0b01101101;		// character code for '5'
	CharacterArray[6] = 0b01111101;		// character code for '6'
	CharacterArray[7] = 0b00000111;		// character code for '7'
	CharacterArray[8] = 0b01111111;		// character code for '8'
	CharacterArray[9] = 0b01101111;		// character code for '9'
	CharacterArray[10] = 0b01110111;	// character code for 'A'
	CharacterArray[11] = 0b01111100;	// character code for 'B'
	CharacterArray[12] = 0b00111001;	// character code for 'C'
	CharacterArray[13] = 0b01011110;	// character code for 'D'
	CharacterArray[14] = 0b01111001;	// character code for 'E'
	CharacterArray[15] = 0b01110001;	// character code for 'F'
}
