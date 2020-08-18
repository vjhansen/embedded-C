//*******************************************************************************
// Project 		LCD - Display a Variable (Embedded C) 
// Target 		ATMEL ATmega1281 micro-controller on STK300 board
// Program		Small_LCD_DisplayVariable_1281_C.c
// Author		Richard Anthony
// Date			11th October 2013 (8535 version 10th October 2011)

// Function		Displays a variable value on the LCD

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
	unsigned char count = 0;
	
	lcd_Clear();			// Clear the display
	lcd_StandardMode();		// Set Standard display mode
	lcd_on();				// Set the display on
	lcd_CursorOff();		// Set the cursor display off (underscore)
	lcd_CursorPositionOff();// Set the cursor position indicator off (flashing square)
	
	lcd_SetCursor(0x00);	// Set cursor position to first line, first column
	lcd_WriteString("Count value is: ");
	
    while(1)        		
    {
		lcd_WriteVariable_withValueAndPositionParameters_SingleDecimalDigit(0 /*row*/, 15 /*column*/, count /* data*/);
		count++;
		if(count > 9)
		{
			count = 0;	// count value runs 0 ... 9 and then back to 0
		}
		_delay_ms(800);
    }
}

void InitialiseGeneral()
{
	DDRA = 0xFF;			// Configure PortA direction for Output
	DDRC = 0xFF;			// Configure PortC direction for Output
	DDRG = 0xFF;			// Configure PortD direction for Output
	
	sei();	// Enable interrupts at global level set Global Interrupt Enable (I) bit
}
