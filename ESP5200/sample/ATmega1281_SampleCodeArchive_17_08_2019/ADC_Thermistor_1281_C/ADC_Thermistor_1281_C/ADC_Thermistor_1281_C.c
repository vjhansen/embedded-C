//*****************************************
// Project		ADC THERMISTOR Demonstration (ADC using Interrupts) (Embedded C)
// Target		ATmega1281 on STK300
// Program		ADC_Thermistor_1281_C.c
// Author		Richard Anthony
// Date			18th October 2013 (original ATmega8535 version 1st November 2011)

// Function		Demonstrates:	ADC operation
//								Use of Interrupts
//								Use of Thermistor

//					Reads ADC channel 6 (Port F bit 6)
//					Displays binary digital value on on-board LEDS


// ***********************************************************************************
// IMPORTANT NOTE - REMOVE JTAG CONNECTOR AFTER PROGRAMMING (USES PORT F BITS 4,5,6,7)
// ***********************************************************************************


//					ADC operates in 'free run mode'
//					(Auto re-starts conversion at end of previous conversion)
//					Triggers interrupt when conversion is complete

// I/O hardware	Uses custom-built Thermistor analog input sensor on Port F
//					(Thermistor is connected as part of voltage divider across GND and VCC)	
//					The thermistor is a negative temperature coefficient device, so resistance
//					falls as temperature rises. The thermistor is connected as the +ve side of
//					the voltage divider so that the voltage, and thus ADC output value, rises
//					the temperature rises.
//					Typical digital values (as decimal)
//					
//					Ice water (0 Celcius)				079
//					  Cool room (14.5 Celcius)			111
//					  Cool room (15.0 Celcius)			113
//					  Cool room (16.5 Celcius)			118
//					  Boiling water (100 Celcius)		239

//					The Thermistor output voltage spans the entire ADC Minimum to FSD 
//					voltage range and thus the full 10-bit digital number range.
//					Bit 5 of the ADC ADMUX register is thus set - giving the convenience of
//					being able to read the most significant 8 bits of the digital value,
//					mapped onto the single 8-bit register ADCH.

//					IMPORTANT NOTE - when using only the ADCL value (i.e. not in this case), 
//					the ADCH register must still be read and the data discarded. Otherwise the
//					ADCL value is corrupted during the next conversion (this is due to the
//					buffering of the internal 16-bit register used by the ADC).

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
	//	Set ADC multiplexer selection register (ADMUX)
	//	bit7,6 Reference voltage selection
	//	00 = AREF, Internal Vref turned off
	//	01 = AVCC with external capacitor at AREF pin
	//	bit 5 ADC Left adjust the 10-bit result
	//	0 = ADCH (high) contains bit 1 = output bit 9, bit 0 = output bit 8
	//		 ADCL (low) contains output bits 7 through 0
	//	1 = ADCH (high) contains bits 9 through 2
	//	    ADCL (low) contains bit 7 = output bit 1, bit 6 = output bit 0
	//	 bit 4,3,2,1,0 Analog channel and Gain selection
	//	 00000 = ADC0 (ADC channel 0, single-ended input)
	//	 (When bits 4 and 3 are both 0, bits 2-0 indicate which ADC channel is used; single-emnded input)
	//	 00010 = ADC2 (ADC channel 2, single-ended input)
	ADMUX = 0b01100110;		// Use AVCC as voltage ref, Convert channel 6 (Thermistor connected to bit 6)
							// Read most-significant 8 bits via ADCH
							// For Thermistor adjust the 10-bit range, so most significant
							// 8 bits are available in the 8-bit register ADCH

	// Enable and set up ADC via ADC Control and Status Register(ADCSR)
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
	ADCSRA = 0b10101110;	// ADC enabled, Auto trigger, Interrupt enabled, Prescaler = 64

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
	ADCSRB = 0b00000000;	// clear bits 3,2,1,0 (Free running mode)

	// DIDR0 - Digital Input Disable Register 0
	// Bit 7:0 – ADC7D:ADC0D: ADC7:0 Digital Input Disable
	DIDR0 = 0b01000000;	// Disable digital input on bit 6

	// DIDR2 – Digital Input Disable Register 2
	// Bit 7:0 – ADC15D:ADC8D: ADC15:8 Digital Input Disable
	DIDR2 = 0b11111111;	// Disable digital input on all bits (64-pin version of ATmega1281 does not even have these inputs)
}

void Start_ADC_Conversion(void)
{	// Start the ADC Conversion (start first sample, runs in 'free run' mode after)
	//	bit 6 ADSC (ADC Start Conversion) = 1 (START)
	//	Read ADSCSR and OR with this value to set the flag without changing others
	ADCSRA |= 0b01000000;
}

ISR(ADC_vect)	// ADC Interrupt Handler
{
	PORTB = ~ADCH;	// Display the ADC input reading onto the LEDs
}
