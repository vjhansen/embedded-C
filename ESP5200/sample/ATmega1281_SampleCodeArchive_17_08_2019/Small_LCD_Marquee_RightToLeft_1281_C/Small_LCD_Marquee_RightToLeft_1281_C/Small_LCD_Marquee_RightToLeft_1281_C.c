//*******************************************************************************
// Project 		LCD - Marquee - Rotate message from Right To Left (Embedded C) 
// Target 		ATMEL ATmega1281 micro-controller on STK300 board
// Program		Small_LCD_Marquee_RightToLeft_1281_C.c
// Author		Richard Anthony
// Date			11th October 2013 (8535 version 8th October 2011)

// Function		Rotates a message from right to left

// 				Uses port A for Data and Command values (8 bits, Output)
//				The LCD 'device Busy' flag also uses bit 7 of port A 
//				So sometimes this bit has to be set for Input

//				Uses port C for control (register select 'RS' bit 6 Output)
//										(device enable 'ENABLE' bit 7 Output)

//				Uses port G for control (write 'WR' bit 0 Output) 
//										(read 'RD' bit 1 Output) 
//*******************************************************************************

#include <avr/io.h>
#include <avr/interrupt.h>	
#include <util/delay.h>		// Add this header so the _delay_ms() function can be used

#include "LCD_LibraryFunctions_1281.h"

void InitialiseGeneral();

int main( void )
{
	InitialiseGeneral();

	lcd_Clear();			// Clear the display
	lcd_StandardMode();		// Set Standard display mode
	lcd_on();				// Set the display on
	lcd_CursorOff();		// Set the cursor display off (underscore)
	lcd_CursorPositionOff();// Set the cursor position indicator off (flashing square)
	lcd_OneLineMode();		// Set the LCD to operate in single line mode

	lcd_SetCursor(0x00);	// Set cursor position to first line, first column
	lcd_WriteString("                ***** Here is a long message that rotates from right to left *****");
	
    while(1)        		
    {
		lcd_ShiftLeft();
		_delay_ms(500);
    }
}

void InitialiseGeneral()
{
	DDRA = 0xFF;			// Configure PortA direction for Output
	DDRC = 0xFF;			// Configure PortC direction for Output
	DDRG = 0xFF;			// Configure PortG direction for Output
	
	asm volatile ("SEI");	// Enable interrupts at global level set Global Interrupt Enable (I) bit
							// (also provides an example of in-line inclusion of assembly code into 'C' program)
}
