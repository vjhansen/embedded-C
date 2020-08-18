//*******************************************************************************
// Project 		LCD - Multiple Messages - Switch Selected (Embedded C) 
// Target 		ATMEL ATmega1281 micro-controller on STK300 board
// Program		Small_LCD_MultiMessage_SwitchSelect_1281_C.c
// Author		Richard Anthony
// Date			11th October 2013 (8535 version 8th October 2011)

// Function		Displays different strings on the LCD, depending on which switch is pressed

// 				Uses port A for Data and Command values (8 bits, Output)
//				The LCD 'device Busy' flag also uses bit 7 of port A 
//				So sometimes this bit has to be set for Input

//				Uses port C for control (register select 'RS' bit 6 Output)
//										(device enable 'ENABLE' bit 7 Output)

//				Uses port G for control (write 'WR' bit 0 Output) 
//										(read 'RD' bit 1 Output) 
//*******************************************************************************

#include <avr/io.h>			// Default include path is C:\WinAVR\avr\include\avr
#include <avr/interrupt.h>
#include "LCD_LibraryFunctions_1281.h"

void InitialiseGeneral();

int main( void )
{
	unsigned char Switches_Value;
	InitialiseGeneral();

	lcd_Clear();				// Clear the display
	lcd_StandardMode();			// Set Standard display mode
	lcd_on();					// Set the display on
	lcd_CursorOff();			// Set the cursor display off (underscore)
	lcd_CursorPositionOff();	// Set the cursor position indicator off (flashing square)

	lcd_SetCursor(0x00);		// Set cursor position to line 1, col 0
	lcd_WriteString("Press one of the");
	lcd_SetCursor(0x40);		// Set cursor position to line 2, col 0
	lcd_WriteString("switches 0-3");
			
    while(1)
    {	
		Switches_Value = PIND;	// Read value on switches

		if(0 == (Switches_Value & 0b00000001)) // Switches are inverted, so when pressed will give a '0'
		{	// Switch 0 pressed
			lcd_Clear();				// Clear the display
			lcd_SetCursor(0x00);		// 0b00000010	Set cursor position to line 1, col 0
			lcd_WriteChar(0xF8);		// Write a specific character - see the LCD manual for details
			lcd_WriteString("ichard says Hi ");
		}				
		if(0 == (Switches_Value & 0b00000010)) // Switches are inverted, so when pressed will give a '0'
		{	// Switch 1 pressed
			lcd_Clear();				// Clear the display
			lcd_SetCursor(0x00);		// 0b00000010	Set cursor position to line 1, col 0
			lcd_WriteChar(0xF8);		// Write a specific character - see the LCD manual for details
			lcd_WriteString("ichard says Bye");
		}				
		if(0 == (Switches_Value & 0b00000100)) // Switches are inverted, so when pressed will give a '0'
		{	// Switch 2 pressed
			lcd_Clear();				// Clear the display
			lcd_SetCursor(0x02);		// 0b00000010	Set cursor position to line 1, col 2
			lcd_WriteString("Richard says");
			lcd_SetCursor(0x45);		// 0b01000101	Set cursor position to line 2, col 5
			lcd_WriteString("Hello");
		}
		if(0 == (Switches_Value & 0b00001000)) // Switches are inverted, so when pressed will give a '0'
		{	// Switch 3 pressed
			lcd_Clear();				// Clear the display
			lcd_SetCursor(0x02);		// 0b00000010	Set cursor position to line 1, col 2
			lcd_WriteString("Richard says");
			lcd_SetCursor(0x45);		// 0b01000101	Set cursor position to line 2, col 5
			lcd_WriteString("Goodbye");
		}
	}
}

void InitialiseGeneral()
{
	DDRA = 0xFF;			// Configure PortA direction Output
	DDRC = 0xFF;			// Configure PortC direction Output
	DDRG = 0xFF;			// Configure PortG direction Output

	DDRD = 0x00;			// Configure PortD direction Input (Switches)
	
	sei();	// Enable interrupts at global level set Global Interrupt Enable (I) bit
}