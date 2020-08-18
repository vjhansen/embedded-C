//*******************************************************************************
// Project 		LCD - Multiple Messages - determined by ADC channel 0 value (Embedded C) 
// Target 		ATMEL ATmega1281 micro-controller on STK300 board
// Program		Small_LCD_ADC_Diagnostic_1281_C.c
// Author		Richard Anthony
// Date			20th October 2017

// Function		Displays different strings on the LCD, depending on the value of the analogue signal on ADC channel 2
//				Can be used as a diagnostic test with any analogue sensor connected on channel 2 of the ADC
//				The easiest way to see the behaviour is to use a potentiometer input on ADC channel 2

//				Analogue input on port F

// 				LCD uses port A for Data and Command values (8 bits, Output)
//				The LCD 'device Busy' flag also uses bit 7 of port A, so sometimes this bit has to be set for Input

//				LCD uses port C for control (register select 'RS' bit 6 Output)
//										(device enable 'ENABLE' bit 7 Output)

//				LCD uses port G for control (write 'WR' bit 0 Output) 
//										(read 'RD' bit 1 Output) 

//				Displays the raw ADC data onto the LEDs (Port B) for further diagnostic output
//*******************************************************************************

#define F_CPU 1000000
#include <avr/io.h>			// Default include path is C:\WinAVR\avr\include\avr
#include <avr/interrupt.h>
#include <util/delay.h>
#include <string.h>
#include "LCD_LibraryFunctions_1281.h"

void InitialiseGeneral();
void Initialise_ADC();

volatile unsigned char ucADC_RawData;
 
int main( void )
{
	unsigned char ucADC_DataRange; // 0 = data value 0-15, 1 = data value 16-31, 2 = data value 32-47 ... 15 = data value 240-255
	unsigned char ucLCD_DisplayPosition;
	InitialiseGeneral();
	Initialise_ADC();
	
	lcd_Clear();				// Clear the display
	lcd_StandardMode();			// Set Standard display mode
	lcd_on();					// Set the display on
	lcd_CursorOff();			// Set the cursor display off (underscore)
	lcd_CursorPositionOff();	// Set the cursor position indicator off (flashing square)

	lcd_SetCursor(0x00);		// Set cursor position to line 0, col 0
	lcd_WriteString("ADC2 data value:");
			
    while(1)
    {	
		lcd_SetCursor(0x40);		// Set cursor position to line 1, col 0
		lcd_WriteString("                ");  // Clear line 1 to remove previous character
		ucADC_DataRange = ucADC_RawData / 16; // Divide the raw data (range 0-255) by 16 giving result in range 0-15
		ucLCD_DisplayPosition = 0x40 + ucADC_DataRange;		// Set cursor position to line 1, col is determined by value in ucADC_DataRange
		lcd_SetCursor(ucLCD_DisplayPosition);
		lcd_WriteChar(0xF5);		// Write a specific character - see the LCD manual for details (contained in file W07_IntroductionToKeypadAndLiquidCrystalDisplay.ppt)
		_delay_ms(20);				// Short delay since LCD needs time to perform display update
	}
}

void InitialiseGeneral()
{
	DDRB = 0xFF;			// Configure PortB direction for Output (LEDs)
	PORTB = 0xFF;			// Set all LEDs initially off (inverted on the board, so '1' = off)
	
	DDRA = 0xFF;			// Configure PortA direction Output (LCD)
	DDRC = 0xFF;			// Configure PortC direction Output (LCD)
	DDRG = 0xFF;			// Configure PortG direction Output (LCD)

	sei();	// Enable interrupts at global level set Global Interrupt Enable (I) bit
}

void Initialise_ADC()	
{
// ADMUX – ADC Multiplexer Selection Register
// bit7,6 Reference voltage selection (00 AREF,01 AVCC, 10 = Internal 1.1V, 11 = Internal 2.56V)
// bit 5 ADC Left adjust the 10-bit result
// 0 = ADCH (high) contains bit 1 = output bit 9, bit 0 = output bit 8. ADCL (low) contains output bits 7 through 0
// 1 = ADCH (high) contains bits 9 through 2. ADCL (low) contains bit 7 = output bit 1, bit 6 = output bit 0
// Bits 4:0 – MUX4:0: Analog Channel and Gain Selection Bits (see 1281 manual p290)
// 00000 = ADC0 (ADC channel 0, single-ended input)
// 00010 = ADC2 (ADC channel 2, single-ended input)
	ADMUX = 0b01100010;	// AVCC REF, Left-adjust output (Read most-significant 8 bits via ADCH), Convert channel 2

// ADCSRA – ADC Control and Status Register A
// bit 7 ADEN (ADC ENable) = 1 (Enabled)
// bit 6 ADSC (ADC Start Conversion) = 0 (OFF initially)
// bit 5 ADATE (ADC Auto Trigger Enable) = 1 (ON)
// bit 4 ADIF (ADC Interrupt Flag) = 0 (not cleared)
// bit 3 ADIE (ADC Interrupt Enable) = 1 (Enable the ADC Conversion Complete Interrupt)
// bit 2,1,0 ADC clock prescaler
// 000 = division factor 2
// 001 = division factor 2
// 010 = division factor 4
// 011 = division factor 8
// 100 = division factor 16
// 101 = division factor 32
// 110 = division factor 64
// 111 = division factor 128
	ADCSRA = 0b10001111;	// ADC enabled, Interrupt enabled, Prescaler = 128

// ADCSRB – ADC Control and Status Register B
// Bit 3 – MUX5: Analog Channel and Gain Selection Bit (always 0 when using ADC0 - ADC7)
// Bit 2:0 – ADTS2:0: ADC Auto Trigger Source (active when ADATE bit in ADCSRA is set)
// 0 0 0 Free Running mode
// 0 0 1 Analog Comparator
// 0 1 0 External Interrupt Request 0
// 0 1 1 Timer/Counter0 Compare Match A
// 1 0 0 Timer/Counter0 Overflow
// 1 0 1 Timer/Counter1 Compare Match B
// 1 1 0 Timer/Counter1 Overflow
// 1 1 1 Timer/Counter1 Capture Event
	ADCSRB &= 0b11110000;	// clear bits 3,2,1,0 (Free running mode)

// DIDR0 – Digital Input Disable Register 0
// Bit 7:0 – ADC7D:ADC0D: ADC7:0 Digital Input Disable
	DIDR0 = 0b00000100;	// Disable digital input on bit 2

// DIDR2 – Digital Input Disable Register 2
// Bit 7:0 – ADC15D:ADC8D: ADC15:8 Digital Input Disable
	DIDR2 = 0b11111111;	// Disable digital input on all bits (64-pin version of ATmega1281 does not even have these inputs)

// Start the ADC Conversion
	//bit 6 ADCSRA (ADC Start Conversion) = 1 (START)
	// Read ADSCSR and OR with this value to set the flag without changing others
	ADCSRA |= 0b01000000;	// start first ADC conversion	
}
//*********  END of ADC configuration subroutine  ********

ISR(ADC_vect)	// ADC Interrupt Handler
{
	ucADC_RawData = ADCH;	// read ADC raw data
	PORTB = ~ucADC_RawData;	// Display the ADC raw data value onto the LEDs
	ADCSRA |= 0b01000000;	// start next ADC conversion	
}
