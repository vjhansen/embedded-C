//*******************************************
// Project 		Accelerometer 2Axis Custom Module With LED bars
// Target 			ATmega1281 on STK300
// Program			Accelerometer_2AxisWithLEDbars_1281_C.c
// Author			Richard Anthony
// Date			28th September 2013 (Original 13th January 2009)
//
// Function		Demonstrates: 	Accelerometer (As 2D Spirit-Level)
//								ADC operation
//								Use of Interrupts
									
//					Reads ADC channels 0,1 (Port F bits 0,1)
//					Displays binary digital value on on-module LEDS

//					ADC operates in 'free run mode'
//					(Auto re-starts conversion at end of previous conversion)
//					Triggers interrupt when conversion is complete

// I/O hardware	Uses 2-Axis Accelerometer sensor on Port F

//					The Accelerometer output voltage spans the entire ADC Minimum to FSD 
//					voltage range and thus the full 10-bit digital number range.
//					Bit 5 of the ADC ADMUX register is thus set - giving the convenience of
//					being able to read the most significant 8 bits of the digital value,
//					mapped onto the single 8-bit register ADCH.

//					IMPORTANT NOTE - when using only the ADCL value (i.e. not in this case), 
//					the ADCH register must still be read and the data discarded. Otherwise the
//					ADCL value is corrupted during the next conversion (this is due to the
//					buffering of the internal 16-bit register used by the ADC).

//					Uses two On-module LED bars
//					Port B = X Axis (Green)
//					Port C = Y Axis (Red)
//*******************************************
#include <avr/io.h>
#include <avr/interrupt.h>

#define MidPoint 132
#define LeftmostPoint 115
#define RightmostPoint 147

void InitialiseGeneral();
void Initialise_ADC();
void SET_XY_Axis_LOOKUP_TABLE();
void DisplayBar_X_Axis();
void DisplayBar_Y_Axis();

unsigned char DisplayPatternTable[32];
unsigned char ADC_Data;

int main( void )
{
	ADC_Data = 0;
	InitialiseGeneral();
	SET_XY_Axis_LOOKUP_TABLE();
	Initialise_ADC();

    while(1)
    {
    }
}

void InitialiseGeneral()
{
	DDRB = 0xFF;			// Configure PortB direction for Output (GREEN LEDs)
	DDRC = 0xFF;			// Configure PortC direction for Output (RED LEDs)
	
	sei();					// Enable interrupts at global level set Global Interrupt Enable (I) bit
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
	ADMUX = 0b01100000;		// AVCC REF, Left-adjust output (Read most-significant 8 bits via ADCH), Convert channel 0 initially

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
	ADCSRA = 0b10101101;	// ADC enabled, Auto trigger, Interrupt enabled, Prescaler = 32

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
	DIDR0 = 0b00000011;	// Disable digital input on bits 0 and 1

	// DIDR2 – Digital Input Disable Register 2
	// Bit 7:0 – ADC15D:ADC8D: ADC15:8 Digital Input Disable
	DIDR2 = 0b11111111;	// Disable digital input on all bits (64-pin version of ATmega1281 does not even have these inputs)

	// Start the ADC Conversion (start first sample, runs in 'free run' mode after)
	//bit 6 ADCSRA (ADC Start Conversion) = 1 (START)
	// Read ADSCSR and OR with this value to set the flag without changing others
	ADCSRA |= 0b01000000;	// start ADC conversion	
}
//*********  END of ADC configuration subroutine  ********

ISR(ADC_vect)	// ADC Interrupt Handler
{	// This interrupt handler is common for all ADC channels
	// Need to alternate which channel is converted
	unsigned char ADMUX_temp = ADMUX;
	unsigned char ADCH_temp = ADCH;
	
	ADMUX_temp &= 0b00011111;	// Mask off non-multiplexer bits
	if(0b00000000 == ADMUX_temp)
	{
		// ADC1_HANDLER
		// NOTE: the ADC starts its next conversion immediately after the previous one ends,
		// i.e. BEFORE the new value is written to ADMUX telling it to change over.
		// Therefore when the flag is set to a particular value (assuming alternation, as in this code)
		// it means that the current conversion just finished will have been for the non-selected channel
		// and the selected channel conversion is underway at this moment.
		// Thus the logic of the above four statements seems to be reversed, but is actually correct.

		ADMUX = 0b01100001;		// Set ADMUX ADC register - next conversion is for ADC1 (see note below)
		// NOTE: As soon as the most-recent conversion ended, another will have started using the previous value written in ADMUX
		// So the most-recent conversion was for ADC1 (thats why we are in ADC1 handler at present)
		// During the ADC1 conversion, the ADMUX flag was set to ADC0 - this conversion is underway NOW
		// So here we must set the flag for ADC1 once again (we are working with an effective lag of one conversion time)

		ADC_Data = ADCH;	// see notes for ADMUX configuration register, bit 5
		DisplayBar_X_Axis();
	}
	else
	{
		// ADC0_HANDLER
		ADMUX = 0b01100000;		// Set ADMUX ADC register - next conversion is for ADC0 (see note below)
		// NOTE: As soon as the most-recent conversion ended, another will have started using the previous value written in ADMUX
		// So the most-recent conversion was for ADC0 (thats why we are in ADC0 handler at present)
		// During the ADC0 conversion, the ADMUX flag was set to ADC1 - this conversion is underway NOW
		// So here we must set the flag for ADC0 once again (we are working with an effective lag of one conversion time)

		ADC_Data = ADCH;	// see notes for ADMUX configuration register, bit 5
		DisplayBar_Y_Axis();
	}
}

void DisplayBar_X_Axis()
{	// X axis connected to Port B and GREEN LEDs in Non-Inverted configuration
	if(ADC_Data <= LeftmostPoint)
	{	// Min value is 115 (90 degrees to left)
		// The 32 bar display values start at 116
		PORTB = 0xE0;	// MSB on LED bar is to the left
		return;
	}
	
	if(ADC_Data >= RightmostPoint)
	{	// Max value is 147 (90 degrees to Right)
		PORTB = 0x07;	// LSB on LED bar is to the right
		return;
	}

	// Here if between 116 and 146 inclusive
	ADC_Data -= (LeftmostPoint + 1);
	// XDisplay is reversed (due to the direction the port is wired to LEDs, respective to the Accelerometer chip
	PORTB = DisplayPatternTable[31 - ADC_Data];
}

void DisplayBar_Y_Axis()
{	// Y axis connected to Port C and RED LEDs in Non-Inverted configuration
	if(ADC_Data <= LeftmostPoint)
	{	// Min value is 115 (90 degrees to left)
		// The 32 bar display values start at 116
		PORTC = 0x07;	// MSB on LED bar is to the Left
		return;
	}
	
	if(ADC_Data >= RightmostPoint)
	{	// Max value is 147 (90 degrees to Right)
		PORTC = 0xE0;	// LSB on LED bar is to the right
		return;
	}

	// Here if between 116 and 146 inclusive
	ADC_Data -= (LeftmostPoint + 1);
	PORTC = DisplayPatternTable[ADC_Data];
}

void SET_XY_Axis_LOOKUP_TABLE()
{	// both Axes have same voltage range and sensitivity
	// The pattern here gives a 'spirit-level' effect over an 8-LED bar graph
	DisplayPatternTable[0] = 1;
	DisplayPatternTable[1] = 1;
	DisplayPatternTable[2] = 1;
	DisplayPatternTable[3] = 1;

	DisplayPatternTable[4] = 2;
	DisplayPatternTable[5] = 2;
	DisplayPatternTable[6] = 2;
	DisplayPatternTable[7] = 2;

	DisplayPatternTable[8] = 4;
	DisplayPatternTable[9] = 4;
	DisplayPatternTable[10] = 4;
	DisplayPatternTable[11] = 4;

	DisplayPatternTable[12] = 8;
	DisplayPatternTable[13] = 8;
	DisplayPatternTable[14] = 8;

	DisplayPatternTable[15] = 24;
	DisplayPatternTable[16] = 24;

	DisplayPatternTable[17] = 16;
	DisplayPatternTable[18] = 16;
	DisplayPatternTable[19] = 16;

	DisplayPatternTable[20] = 32;
	DisplayPatternTable[21] = 32;
	DisplayPatternTable[22] = 32;
	DisplayPatternTable[23] = 32;

	DisplayPatternTable[24] = 64;
	DisplayPatternTable[25] = 64;
	DisplayPatternTable[26] = 64;
	DisplayPatternTable[27] = 64;

	DisplayPatternTable[28] = 128;
	DisplayPatternTable[29] = 128;
	DisplayPatternTable[30] = 128;
	DisplayPatternTable[31] = 128;
}

