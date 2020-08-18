//*******************************************************************************
// Project 		LCD - Single Hard-Coded Message (Embedded C)
// Target 		ATMEL ATmega1281 micro-controller on STK300 board
// Program		Small_LCD_FixedMessage_PortsACG_1281_C.c
// Author		Richard Anthony
// Date			9th October 2013

// Function		Displays a fixed string on the Small LCD

// NOTE - This sample code works with the small LCD modules that are connected via the dedicated LCD connector
//		(the connector is identified by having a single row of 14 pins)

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

	lcd_SetCursor(0x02);		// 0b00000010	Set cursor position to line 1, col 2
	lcd_WriteChar(0xF8);		// Write a specific character - see the LCD manual for details
	lcd_WriteString("ichard says");
				
	lcd_SetCursor(0x41);		//.0b01000101	Set cursor position to line 2, col 1
	lcd_WriteString("Hello on 1281!");
	
    while(1)
    {
    }
}

void InitialiseGeneral()
{
	DDRA = 0xFF;			// Configure PortA direction for Output
	DDRC = 0xFF;			// Configure PortC direction for Output
	DDRG = 0xFF;			// Configure PortG direction for Output
	
	sei();					// Enable interrupts at global level set Global Interrupt Enable (I) bit
}