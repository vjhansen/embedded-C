//*****************************************
// Project 		ADC PHOTODIODE Demonstration (ADC using Interrupts) (Embedded C)
// Target 		ATmega1281 on STK300
// Program		ADC_Photodiode_1281_C.c
// Author		Richard Anthony
// Date			18th October 2013 (original ATmega8535 version 1st November 2011)

// Function		Reads ADC channel 7 (Port F bit 7, connector pin 7)
//					Displays binary digital value on on-board LEDS

//					ADC operates in 'free run mode'
//					(Auto re-starts conversion at end of previous conversion)
//					Triggers interrupt when conversion is complete

// I/O hardware	Uses custom-built PHOTODIODE analog test input rig on Port F
//					(BPW21 Photodiode connected across GND and VCC)

//					The PHOTODIODE only uses a small part of the voltage range when
//					connected directly across GND and the ADC input.
//					Therefore, do NOT adjust the 10-bit range (bit 5 of ADMUX),
//					Use as is to get maximum differentiation from the lower voltage change
//					in a single 8-bit register ADCL
//					IMPORTANT NOTE - when using only the ADCL value, the ADCH register must
//					still be read and the data discarded. Otherwise the ADCL value is
//					corrupted during the next conversion.
//					(NOTE, For the POTENTIOMETER which gives the full voltage range and thus
//					the full 10-bit digital number range, it was more convenient to set bit 5
//					of the ADC ADMUX register, and read only the most significant 8 bits of
//					the digital value, mapped onto the single 8-bit register ADCH).

//					Port B is set for output (on-board LEDs)
//*****************************************

#include <avr/io.h>
#include <avr/interrupt.h>

void InitialiseGeneral();
void Initialise_ADC();
void Start_ADC_Conversion(void);

int main( void )
{
	InitialiseGeneral();
	Initialise_ADC();
	Start_ADC_Conversion();		// Start ADC conversion (only need to do this the first time
								// because 'free running' mode is used)
	while(1)
	{
	}
}

void InitialiseGeneral()
{
	DDRB = 0xFF;			// Configure PortB direction for Output
	PORTB = 0xFF;			// Set all LEDs initially off (inverted on the board, so '1' = off)
	
	sei();	// Enable interrupts at global level set Global Interrupt Enable (I) bit
}

void Initialise_ADC()
{
	// Set ADC multiplexer selection register (ADMUX)
	//	bit7,6 Reference voltage selection
	//	00 = AREF, Internal Vref turned off
	//	01 = AVCC with external capacitor at AREF pin
	//	bit 5 ADC Left adjust the 10-bit result
	//	0 = ADCH (high) contains bit 1 = output bit 9, bit 0 = output bit 8
	//		 ADCL (low) contains output bits 7 through 0
	//	1 = ADCH (high) contains bits 9 through 2
	//	    ADCL (low) contains bit 7 = output bit 1, bit 6 = output bit 0
	//	 bit 4,3,2,1,0 Analog channel and Gain selection (see 8535 manual p220)
	//	 00000 = ADC0 (ADC channel 0, single-ended input)
	//	 (When bits 4 and 3 are both 0, bits 2-0 indicate which ADC channel is used; single-emnded input)
	//	 00010 = ADC2 (ADC channel 2, single-ended input)
	ADMUX = 0b01100111;		// Use AVCC as voltage ref, Convert channel 7 (Photodiode connected to bit 7)
	// Read most-significant 8 bits via ADCH
	// For Thermistor adjust the 10-bit range, so most significant
	// 8 bits are available in the 8-bit register ADCH

	//Enable and set up ADC via ADC Control and Status Register(ADCSR)
	//(note, header file discrepancy: ADCSR register is named ADCSRA in the header file)
	//	bit 7 ADEN (ADC ENable) = 1 (Enabled)
	//	bit 6 ADSC (ADC Start Conversion) = 0 (OFF initially)
	//	bit 5 ADATE (ADC Auto Trigger Enable) = 1 (ON)
	//	bit 4 ADIF (ADC Interrupt Flag) = 0 (not cleared)
	//	bit 3 ADIE (ADC Interrupt Enable) = 1 (Enable the ADC Conversion Complete Interrupt)
	//	bit 2,1,0 ADC clock prescaler
	//	000 = division factor 2
	//	001 = division factor 2
	//	010 = division factor 4
	//	011 = division factor 8
	//	100 = division factor 16
	//	101 = division factor 32
	//	110 = division factor 64
	//	111 = division factor 128
	ADCSRA = 0b10101110; // ADC enabled, Auto trigger, Interrupt enabled, Prescaler = 64

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
	ADCSRB = 0b11110000;		// clear bits 3,2,1,0 (Free running mode)

	// DIDR0 - Digital Input Disable Register 0
	// Bit 7:0 – ADC7D:ADC0D: ADC7:0 Digital Input Disable
	DIDR0 = 0b10000000;	// Disable digital input on bit 7

	// DIDR2 – Digital Input Disable Register 2
	// Bit 7:0 – ADC15D:ADC8D: ADC15:8 Digital Input Disable
	DIDR2 = 0b11111111;	// Disable digital input on all bits (64-pin version of ATmega1281 does not even have these inputs)
}

void Start_ADC_Conversion(void)
{	//Start the ADC Conversion (start first sample, runs in 'free run' mode after)
	//	bit 6 ADSC (ADC Start Conversion) = 1 (START)
	//	Read ADSCSR and OR with this value to set the flag without changing others
	unsigned char New_ADCSRA;
	New_ADCSRA = ADCSRA | 0b01000000;
	ADCSRA = New_ADCSRA;
}

ISR(ADC_vect)	// ADC Interrupt Handler
{
	PORTB = ~ADCH;	// Display the ADC input reading onto the LEDs
}