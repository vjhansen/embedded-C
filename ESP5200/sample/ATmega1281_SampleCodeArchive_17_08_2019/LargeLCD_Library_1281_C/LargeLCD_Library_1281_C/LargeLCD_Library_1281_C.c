/*******************************************
Project 		Function library for the 'new, Large' LCD Display MC44005A6W-BNMLW
Target 			ATmega1281 on STK300
Program			LargeLCD_Library_1281_C.c
Author			Richard Anthony
Date			19th September 2013

Fuse settings	System clock: Use the internal RC oscillator at 8.0MHz and CKDIV8 fuse programmed, resulting in 1.0MHz system clock.
				Fuse settings should be: CKSEL = "0010", SUT = "10", CKDIV8 = "0"

Function		Function library for the 'new, Large' LCD Display  Part number MC44005A6W-BNMLW

LCD Display characteristics
				40 characters * 4 lines
				%V operating Voltage (will not work at 3.3V)

Interface configuration
	PortA			-	LCD data bus
	PortC			-	LCD control bus
	Port C bit 0	-	Enable 1 (E1)
	Port C bit 1	-	Read / Write (RW)
	Port C bit 2	-	Register Select (RS)
	Port C bit 3	-	Enable 2 (E2)
*******************************************/
#include <avr/io.h>
#include <util/delay.h>

#define UC unsigned char

#define LCD_INTERFACE_INACTIVE (UC) 0b11110000
#define LCD_READ (UC) 0b00000010
#define LCD_WRITE (UC) 0b00000000
#define LCD_RS_CONTROL (UC) 0b00000000	// Register 0 is for Control Commands
#define LCD_RS_DATA (UC) 0b00000100		// Register 1 is for Data
#define LCD_ENABLE (UC) 0b00001001
#define LCD_NOT_ENABLE (UC) 0b11110110

char flags;


void InitialiseGeneral();
unsigned char uCountvalue;	// Create the variable that will hold the count value

void LCD_Initialise();



void InitialiseGeneral()
{
	// Configure Ports
	DDRB = 0xFF;	// Set port B direction OUTPUT (connected to the on-board LEDs)
	PORTB = 0xFF;	// Set all LEDs initially off (inverted on the board, so '1' = off)

	DDRA = 0xFF;	// Set port A direction OUTPUT (LDC data bus)
	PORTA = 0x00;
	DDRC = 0xFF;	// Set port C direction OUTPUT (LDC control bus)
					// Port C bit 0	-	Enable 1 (E1)
					// Port C bit 1	-	Read / Write (RW)
					// Port C bit 2	-	Register Select (RS)
					// Port C bit 3	-	Enable 2 (E2)
	PORTC = 0x00;
	
	
	uCountvalue  = 0;	// Initialize the count value to 0
}

int main( void )
{
	InitialiseGeneral();


	LCD_Initialise();
	LCD_SendCommandOrData(LCD_RS_CONTROL, LCD_WRITE, 0b01000000); // SET CGRAM address = 0
	LCD_SendCommandOrData(LCD_RS_CONTROL, LCD_WRITE, 0b10000000); // SET DDRAM address = 0
	
	while(1)
	{
	LCD_SendCommandOrData(LCD_RS_DATA, LCD_WRITE, uCountvalue);

		PORTB = ~uCountvalue;	// Write the data value to Port B (the ~ performs 1s compliment
								//// because the switch values are inverted in the STK300 board hardware)
	//
		uCountvalue++;			// Increment the count value
		_delay_ms(50);			// Add a delay so we can see the pattern change (try removing this delay)
	}
}

void LCD_SendCommandOrData(unsigned char uRS_value, unsigned char uRW_value, unsigned char uCommandOrData)
{
	// The LCD interface is timing sensitive, thus this function should be used in all cases
	PORTA = uCommandOrData;				// Set the command/data value so it is stable before use
	PORTC &= LCD_INTERFACE_INACTIVE;	// Clear the LCD interface to a known initial state
	PORTC |= (uRS_value | uRW_value);	// Setup RS and R/W signals
	_delay_ms(10);						// wait >= 40 ns
	PORTC |= LCD_ENABLE;				// Setup Enable signals
	_delay_ms(10);						// wait >= 230 ns
	PORTC &= LCD_NOT_ENABLE;			// Clear Enable signals
	_delay_ms(10);						// wait >= 10 ns
	PORTC &= LCD_INTERFACE_INACTIVE;	// Clear the LCD interface to a known final state

	// Note, the inclusion of > 1 ms of delay in this function guarantees that the Enable cycle time of at least 500ns is respected in all cases
}

void LCD_Initialise()
{	// Meets specific timing sequence specified in datasheet
	_delay_ms(20);
	LCD_SendCommandOrData(LCD_RS_CONTROL, LCD_WRITE, 0b00111111);
	_delay_ms(10);
	LCD_SendCommandOrData(LCD_RS_CONTROL, LCD_WRITE, 0b00111111);
	_delay_ms(10);
	LCD_SendCommandOrData(LCD_RS_CONTROL, LCD_WRITE, 0b00111111);
	
	LCD_SendCommandOrData(LCD_RS_CONTROL, LCD_WRITE, 0b00111100);	// 2 lines, High-res font
	LCD_SendCommandOrData(LCD_RS_CONTROL, LCD_WRITE, 0b00001000);	// Display off
	LCD_SendCommandOrData(LCD_RS_CONTROL, LCD_WRITE, 0b00000001);	// Display clear
	LCD_SendCommandOrData(LCD_RS_CONTROL, LCD_WRITE, 0b00000111);	// Entry mode - shift right
}