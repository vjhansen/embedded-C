//*********************************************************************************************************************
// Project 		LCD - Simple demonstration using custom LCD library (Embedded C)
// Target 		ATMEL ATmega2560 micro-controller and 1602A LCD (as supplied in the HBV sensors kit)
// Program		LCD_1602A_FixedMessage_PortsAG_2560_C.c
// Author		Richard Anthony
// Date			16th October 2016

// Function		Displays the entire character set on the upper row of the LCD
//				Displays a fixed string on the lower row of the LCD

// 	Ports		Uses port A for Data and Command values (8 bits, Output)
//				The LCD 'device Busy' flag also uses bit 7 of port A so sometimes this bit has to be set for Input

//				Uses port G for control		Register Select		Data mode(H)/Command mode(L)		Port G bit 0 Output
//											Data direction		Read(H)/Write(L)					Port G bit 1 Output
//											Enable				Pulse high while read/write occurs	Port G bit 2 Output
//*********************************************************************************************************************

#include <avr/io.h>
#include <avr/interrupt.h>
#define F_CPU 1000000UL
#include <util/delay.h>
#include "LCD_LIB.h"

void InitialiseGeneral();
void InitialiseTimer3();
void Initialise_ADC();
void Start_ADC_Conversion(void);

volatile unsigned char sec_cnt = '0';
volatile unsigned char next_digit = '0';
volatile unsigned char g_uData;

int main(void)
{
	InitialiseGeneral();
	InitialiseTimer3();
	Initialise_ADC();
	Start_ADC_Conversion();

	// Simple demonstration of LCD configuration and data output
	// *********************************************************
	unsigned char cData = '0'; // '0'=30, 'A'=65, 'Z'=90
	int iCharacterPositionCount = 0;
	LCD_ShiftDisplay(false /*true = ON false = OFF*/, true /*true = shift right, false = shift left*/);
	LCD_Display_ON_OFF(true /*Display ON*/, false /*Cursor OFF*/, false /*Cursor Position OFF*/);

	// Display a fixed message on the bottom row of the LCD
	LCD_Clear();
	LCD_Home();
	
	//LCD_WriteString("Burglar Alarm System");
	//LCD_SetCursorPosition(0, 1);
	//LCD_WriteString("Enter code:");
	
	while(1)
    {	
		/*
		if (sec_cnt > '9')
		{
			sec_cnt = '0';
			next_digit++;
		}
		if (next_digit > '9')
		{
			next_digit = '0';
			// add a third digit
		}
		
	
		LCD_Clear();
		LCD_SetCursorPosition(1, 1);
		LCD_WriteChar(sec_cnt);
		LCD_SetCursorPosition(0, 1);
		LCD_WriteChar(next_digit);
		*/
	
		if (g_uData < 120)
		{
			LCD_Clear();
			LCD_SetCursorPosition(0, 0);
			LCD_WriteString("Dark");
		}
		else 
		{
			LCD_Clear();
			LCD_SetCursorPosition(0, 0);
			LCD_WriteString("Light");
		}
		
		iCharacterPositionCount++;
		if(iCharacterPositionCount >= LCD_DisplayWidth_CHARS)
		{	// Reset current position to zero when the end of the display is reached
			LCD_Home();
			iCharacterPositionCount = 0;
		}
		_delay_ms(200);
    }
}

void InitialiseGeneral()
{
	LCD_Initilise(true/*false = 1 line mode, true = 2 line mode*/, false/*false = 5*8pixels, true = 5*11 pixels*/);
	// Includes:	Configuration of PortG direction for Output and PortA direction for Output
	//				Enable the LCD device
	//				Configure 1/2 line mode and small/large font size
	
	sei();			// Enable interrupts at global level set Global Interrupt Enable (I) bit
}



void InitialiseTimer3()     // Configure to generate an interrupt after a 1 Second interval
{
	TCCR3A = 0x00;  // Normal port operation (OC1A, OC1B, OC1C), Clear Timer on 'Compare Match' (CTC) waveform mode)
	TCCR3B = 0b00001101;    // CTC waveform mode, use prescaler 1024
	TCCR3C = 0x00;
	
	// For 1 MHz clock (with 1024 prescaler) to achieve a 60 second interval:
	// Need to count 60*1,000,000 clock cycles (but already divided by 1024)
	// So actually need to count to (1,000,000 / 1024) = 977 decimal, = 3D1 Hex
	//OCR3AH = 0xE4; // Output Compare Registers (16 bit) OCR1BH and OCR1BL // 1 min
	//OCR3AL = 0xE2;
	
	OCR3AH = 0x03;
	OCR3AL = 0xD1;

	TCNT3H = 0x00;  // Timer/Counter count/value registers (16 bit) TCNT3H and TCNT3L
	TCNT3L = 0x00;
	TIMSK3 = 0b00000010;    // bit 1 OCIE3A     Use 'Output Compare A Match' Interrupt, i.e. generate an interrupt
	// when the timer reaches the set value (in the OCR3A registers)
}

ISR(TIMER3_COMPA_vect) // TIMER3_Overflow_Handler (Interrupt Handler for Timer 3) -- 1 sec
{
	sec_cnt++;
}




void Initialise_ADC()
{
	// Set ADC multiplexer selection register (ADMUX)
	// Bit 7:6 - REFS1:0: Reference Selection Bits
	//    01 = AVCC with external capacitor at AREF pin
	
	// Bit 5 - ADLAR: ADC Left Adjust Result
	//  0 = ADCH (high) contains bit 1 = output bit 9, bit 0 = output bit 8
	//    ADCL (low) contains output bits 7 through 0
	//  1 = ADCH (high) contains bits 9 through 2
	//      ADCL (low) contains bit 7 = output bit 1, bit 6 = output bit 0
	
	// Bits 4:0 - MUX4:0: Analog Channel and Gain Selection Bits
	//  00000 = ADC0 (ADC channel 0, single-ended input)
	//  (When bits 4 and 3 are both 0, bits 2-0 indicate which ADC channel is used; single-emnded input)
	//  00010 = ADC2 (ADC channel 2, single-ended input)
	ADMUX = 0b01100000;    // Use AVCC as voltage ref, left adjust (need 8 MSBs) Convert channel 0 (LDR connected to bit 0)

	//Enable and set up ADC via ADC Control and Status Register(ADCSR)
	//(note, header file discrepancy: ADCSR register is named ADCSRA in the header file)
	//  Bit 7 - ADEN: ADC Enable = 1
	//  Bit 6 - ADSC: ADC Start Conversion = 0
	//  Bit 5 - ADATE: ADC Auto Trigger Enable = 1
	//  Bit 4 - ADIF: ADC Interrupt Flag = 0
	//  Bit 3 - ADIE: ADC Interrupt Enable = 1
	//  Bits 2:0 - ADPS2:0: ADC Prescaler Select Bits = 110 (division factor 64)
	ADCSRA = 0b10101110; // ADC enabled, Auto trigger, Interrupt enabled, Prescaler = 64

	// ADCSRB - ADC Control and Status Register B
	// Bit 2:0 - ADTS2:0: ADC Auto Trigger Source = 000 (free running mode)
	// ADCSRB = 0b11110000;    // clear bits 3,2,1,0 (Free running mode)
	ADCSRB &= ~((1 << ADTS2) | (1 << ADTS1) | (1 << ADTS0));

	// DIDR0 - Digital Input Disable Register 0
	//  Bit 7:0 - ADC7D:ADC0D: ADC7:0 Digital Input Disable
	// DIDR0 = 0b00000001;  // Disable digital input on bit 0
	DIDR0 |= (1 << ADC0D);

	// DIDR2 - Digital Input Disable Register 2
	//  Bit 7:0 - ADC15D:ADC8D: ADC15:8 Digital Input Disable
	DIDR2 = 0xFF;  // Disable digital input on all bits
}

void Start_ADC_Conversion(void)
{  // Start the ADC Conversion (start first sample, runs in 'free run' mode after)
	//  bit 6 ADSC (ADC Start Conversion) = 1 (START)
	//  Read ADSCSR and OR with this value to set the flag without changing others
	unsigned char New_ADCSRA;
	New_ADCSRA = ADCSRA | 0b01000000;
	ADCSRA = New_ADCSRA;
}

ISR(ADC_vect)  // ADC Interrupt Handler
{
	// After the conversion is complete, the conversion result can be found in the ADC Result Registers (ADCL, ADCH).
	g_uData = ADCH;   // Copy the output data from the A-to-D converter to a global variable

}