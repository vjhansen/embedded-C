//*******************************************************************************
// Project 		LCD - Bar graph (single) - value input from switches (Embedded C) 
// Target 		ATMEL ATmega1281 micro-controller on STK300 board
// Program		Small_LCD_BarGraphSingle_SwitchInput_1281_C.c
// Author		Richard Anthony
// Date			11th October 2013 (8535 version 10th October 2011)

// Function		Displays a bar chart on the LCD

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
	unsigned char Switches_Value;
	InitialiseGeneral();

	lcd_Clear();			//Clear the display
	lcd_StandardMode();		// Set Standard display mode
	lcd_on();				// Set the display on
	lcd_CursorOff();		// Set the cursor display off (underscore)
	lcd_CursorPositionOff();// Set the cursor position indicator off (flashing square)

    while(1)
    {	
		Switches_Value = PIND;			// Read value on switches
		Switches_Value = ~Switches_Value; // Invert all bits (1's Complement)
		Switches_Value &= 0b00001111;	// Mask off the upper 4 switches  (i.e. values  0 - 15)
		lcd_BarGraph(Switches_Value, 0);
		_delay_ms(300);
    }
}

void InitialiseGeneral()
{
	DDRA = 0xFF;			// Configure PortA direction Output
	DDRC = 0xFF;			// Configure PortC direction Output
	DDRG = 0xFF;			// Configure PortG direction Output
	
	sei();	// Enable interrupts at global level set Global Interrupt Enable (I) bit
}
