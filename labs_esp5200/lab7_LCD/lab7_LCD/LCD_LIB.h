//*******************************************************************************
// Project 		LCD Library Functions header (Embedded C) for LCD module 1602A (as in HBV sensors kit)
// Target 		ATMEL ATmega2560 (on Arduino Mega board)
// Program		LCD_LibraryFunctions_2560.h
// Author		Richard Anthony
// Date			16th October 2016

// Function		Provides low-level functions to configure the LCD

// Ports

// 				Uses port A for Data and Command values (8 bits, Output)
//				LCD module pin		Atmega2560 Pin		Arduino Mega Board Pin
//				**************************************************************
//				D0					Port A0				22
//				D1					Port A1				23
//				D2					Port A2				24
//				D3					Port A3				25
//				D4					Port A4				26
//				D5					Port A5				27
//				D6					Port A6				28
//				D7					Port A7				29

// 				Uses port G for Control (3 bits, Output)
//				LCD module pin		Atmega2560 Pin		Arduino Mega Board Pin		Operation
//				*****************************************************************************
//				Register Select		Port G0				41							Data input H, Command input L
//				Read/Write			Port G1				40							Read H, Write L
//				Enable				Port G2				39							Pulse High to enable the LCD device while read or write occurs

//				Power and ancillary pins
//				LCD module pin	Function
//				************************
//				Vss				0V
//				Vdd				5V
//				V0				Contrast control, connect to centre wiper pin of potentiometer (10K Ohms works fine)
//				A				Backlight LED anode (connect to 5V via 1K Ohm resistor)
//				K				Backlight LED cathode (connect to 0V)


//*******************************************************************************

#include <stdbool.h>
#include <string.h>

#define LCD_DisplayWidth_CHARS 16

//Function declarations
// *** 'Private' Functions accessed by other member functions - do not call these direct from application code ***
void LCD_Write_CommandOrData(bool bCommand /*true = Command, false = Data*/, unsigned char DataOrCommand_Value);
void LCD_Wait();
void LCD_Enable();
void LCD_Disable();
// *** END of 'Private' Functions accessed by other member functions - do not call these direct from application code ***

// *** USER functions
void LCD_Initilise(bool bTwoLine/*false = 1 line mode, true =  2 line mode*/, bool bLargeFont/*false = 5*8pixels, true = 5*11 pixels*/);
void LCD_Display_ON_OFF(bool bDisplayON /*true = ON, false = OFF*/, bool bCursorON, bool bCursorPositionON); // Turn the LCD display ON / OFF
void LCD_Clear();
void LCD_Home();
void LCD_WriteChar(unsigned char cValue);
void LCD_ShiftDisplay(bool bShiftDisplayON /*true = On false = OFF*/, bool bDirectionRight /*true = shift right, false = shift left*/);
void LCD_SetCursorPosition(unsigned char iColumnPosition /*0 - 40 */, unsigned char iRowPosition /*0 for top row, 1 for bottom row*/);
void LCD_WriteString(char Text[]);
// *** END of USER functions

// Function implementations
// *** 'Private' Functions accessed by other member functions - do not call these direct from application code ***
void LCD_Write_CommandOrData(bool bCommand /*true = Command, false = Data*/, unsigned char DataOrCommand_Value)
{
	LCD_Wait();	// Wait if LCD device is busy

	// The access sequence is as follows:
	// 1. Set command lines as necessary:
	if(true == bCommand)
	{	// Register Select LOW, and R/W direction LOW
		PORTG &= 0b11111100;	// Clear Read(H)/Write(L) (PortG bit1),	Clear Register Select for command mode (PortG bit0)
	}
	else /*Data*/
	{	// Register Select HIGH, and R/W direction LOW
		PORTG |= 0b00000001;	// Set Register Select HIGH for data mode (PortG bit0)
		PORTG &= 0b11111101;	// Clear Read(H)/Write(L) (PortG bit1)
	}
	// 2. Set Enable High
	// 3. Write data or command value to PORTA
	// 4. Set Enable Low
	LCD_Enable();
	DDRA = 0xFF;					// Configure PortA direction for Output
	PORTA = DataOrCommand_Value;	// Write combined command value to port A
	LCD_Disable();
}

void LCD_Wait()		// Check if the LCD device is busy, if so wait
{					// Busy flag is mapped to data bit 6, so read as port A bit 6
	PORTG &= 0b11111110;		// Clear Register Select for command mode (PortG bit0)
	PORTG |= 0b00000010;		// Set Read(H)/Write(L) (PortG bit1)
	DDRA = 0x00;				// Configure PortA direction for Input (so busy flag can be read)
	
	unsigned char PINA_value = 0;
	while(PINA_value & 0b10000000);	// Wait here until busy flag is cleared
	{
		LCD_Enable();
		PINA_value = PINA;
		LCD_Disable();
	}
}

void LCD_Enable()
{
	PORTG |= 0b00000100;	// Set LCD Enable (PortG bit2)
}

void LCD_Disable()
{
	PORTG &= 0b11111011;	// Clear LCD Enable (PortG bit2)
}
// *** END of 'Private' Functions accessed by other member functions - do not call these direct from application code ***

// *** USER functions
void LCD_Initilise(bool bTwoLine/*false = 1 line mode, true =  2 line mode*/, bool bLargeFont/*false = 5*8pixels, true = 5*11 pixels*/)
{	// Note, in 2-line mode must use 5*8 pixels font
	
	// Set Port A and Port G for output
	DDRG = 0xFF;		// Configure PortG direction for Output
	PORTG = 0x00;		// Clear port G
	DDRA = 0xFF;		// Configure PortA direction for Output
	PORTA = 0x00;		// Clear port A

	unsigned char Command_value = 0b00110000;	// bit 5 'Function Set' command, bit 4 High sets 8-bit interface mode
	if(true == bTwoLine)
	{
		Command_value |= 0b00001000;	// bit 3 high = 2-line mode (low = 1 line mode)
	}
	else
	{	// One-line mode
		if(true == bLargeFont)
		{	// Large font (nested because can only use large font in one-line mode)
			Command_value |= 0b00000100;	// bit 2 high = large font mode 5*11 pixels (low = small font 5*8pixels)
		}
	}

	LCD_Write_CommandOrData(true /*true = Command, false = Data*/, Command_value);
}

void LCD_Display_ON_OFF(bool bDisplayON /*true = ON, false = OFF*/, bool bCursorON, bool bCursorPositionON) // Turn the LCD display ON / OFF
{
	if(true == bDisplayON)
	{
		if(true == bCursorON)
		{
			if(true == bCursorPositionON)
			{
				// 'Display ON/OFF' function command, Display ON, Cursor ON, Cursor POSITION ON
				LCD_Write_CommandOrData(true /*true = Command, false = Data*/, 0b00001111);
			}
			else
			{
				// 'Display ON/OFF' function command, Display ON, Cursor ON, Cursor POSITION OFF
				LCD_Write_CommandOrData(true /*true = Command, false = Data*/, 0b00001110);
			}
		}
		else /*Cursor OFF*/
		{
			if(true == bCursorPositionON)
			{
				// 'Display ON/OFF' function command, Display ON, Cursor OFF, Cursor POSITION ON
				LCD_Write_CommandOrData(true /*true = Command, false = Data*/, 0b00001101);
			}
			else
			{
				// 'Display ON/OFF' function command, Display ON, Cursor OFF, Cursor POSITION OFF
				LCD_Write_CommandOrData(true /*true = Command, false = Data*/, 0b00001100);
			}
		}
	}
	else
	{
		// 'Display ON/OFF' function command, Display OFF, Cursor OFF, Cursor POSITION OFF
		LCD_Write_CommandOrData(true /*true = Command, false = Data*/, 0b00001000);
	}
}

void LCD_Clear()			// Clear the LCD display
{
	LCD_Write_CommandOrData(true /*true = Command, false = Data*/, 0b00000001);
	_delay_ms(2);	// Enforce delay for specific LCD operations to complete
}

void LCD_Home()			// Set the cursor to the 'home' position
{
	LCD_Write_CommandOrData(true /*true = Command, false = Data*/, 0b00000010);
	_delay_ms(2);	// Enforce delay for specific LCD operations to complete
}

void LCD_WriteChar(unsigned char cValue)
{	// Write character in cValue to the display at current cursor position (position is incremented after write)
	LCD_Write_CommandOrData(false /*true = Command, false = Data*/, cValue);
}

void LCD_ShiftDisplay(bool bShiftDisplayON /*true = On false = OFF*/, bool bDirectionRight /*true = shift right, false = shift left*/)
{
	if(true == bShiftDisplayON)
	{
		if(true == bDirectionRight)
		{
			LCD_Write_CommandOrData(true /*true = Command, false = Data*/, 0b00000101);
		}
		else /*shift display left*/
		{
			LCD_Write_CommandOrData(true /*true = Command, false = Data*/, 0b00000111);
		}
	}
	else /*ShiftDisplay is OFF*/
	{
		LCD_Write_CommandOrData(true /*true = Command, false = Data*/, 0b00000100);
	}
}

void LCD_SetCursorPosition(unsigned char iColumnPosition /*0 - 40 */, unsigned char iRowPosition /*0 for top row, 1 for bottom row*/)
{
	// Cursor position is achieved by repeatedly shifting from the home position.
	// In two-line mode, the beginning of the second line is the 41st position (from home position)
	unsigned char iTargetPosition = (40 * iRowPosition) + iColumnPosition;
	LCD_Home();
	for(unsigned char iPos = 0; iPos < iTargetPosition; iPos++)
	{
		LCD_Write_CommandOrData(true /*true = Command, false = Data*/, 0b00010100); // Shift cursor left one place
	}
}

void LCD_WriteString(char Text[])
{
	for(unsigned char iIndex = 0; iIndex < strlen(Text); iIndex++)
	{
		LCD_WriteChar(Text[iIndex]);
	}
}
// *** END of USER functions
