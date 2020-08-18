//*****************************************
// Project 		Analogue Comparator Demo Using twin Variable Resistors (Embedded C)
// Target 		ATmega1281 on STK300
// Program		AnalogueComparator_1281_C.c
// Author		Richard Anthony
// Date			17th October 2013 (original ATmega8535 assembly version 13th November 2008)

//** Function		Uses the Analogue Comparator to compare two analogue values
//					These are provided on Port B pins as the Alternate Function
//					signals AIN0 (PORT E bit 2, Positive input) and AIN1 (PORT E bit 3, Negative input)

//					When the voltage on the AIN0 input is higher the Analogue Comparator
//					sets the ACO bit in its ACSR register.
//					In this example, when the value of ACO is '1' all the LEDs are turned ON.
//					When the value of ACO is '0' all the LEDs are turned OFF.

//					Although this example polls ACO instead of using the Interrupt generation
//					facility of the Analogue Comparator, the Interrupt handler is setup and enabled
//					to provide a skeleton solution for further development

// I/O hardware	Uses custom-built 2-potentiometer (variable resistor) analog input rig on
//					Port E (2 * 220K-ohm linear potentiometers with track connected
//					across GND and VCC so sweeper forms a voltage divider)
//**					Potentiometer #1 data value is connected to Port E pin2
//**					Potentiometer #2 data value is connected to Port E pin3

//**				Port B is set for output (on-board LEDs)
//*****************************************

#include <avr/io.h>
#include <avr/interrupt.h>
#include <util/delay.h>

void InitialiseGeneral();
void Initialise_AnalogueComparator();

int main( void )
{
	InitialiseGeneral();
	Initialise_AnalogueComparator();
	
    while(1)
    {
		// Display pattern on LEDs to indicate which voltage is higher
		// Polls Analogue Comparator output directly
		if(ACSR & 0b00100000)	// ACSR Bit 5 is ACO: Analog Comparator Output
		{
			PORTB = 0b11111000;	// LEDs 0,1,2 on when Voltage higher on positive input (AIN0) PORT E bit 2
		}
		else
		{
			PORTB = 0b11000111;	// LEDs 3,4,5 on when Voltage higher on negative input (AIN1) PORT E bit 3
		}				
    }
}

void InitialiseGeneral()
{
	DDRB = 0xFF;			// Configure PortB direction for Output (LEDs)
	PORTB = 0xFF;			// Set all LEDs initially off (inverted on the board, so '1' = off)	
	
	sei();					// Enable interrupts at global level set Global Interrupt Enable (I) bit
}

void Initialise_AnalogueComparator()
{
// ADCSRB – ADC Control and Status Register B
	// Bit 6 – ACME: Analog Comparator Multiplexer Enable - set to 0 when using AIN1 as -Ve input
	unsigned char ADCSRB_temp;
	ADCSRB_temp = ADCSRB & 0b10111111; // Clear bit 6
	ADCSRB = ADCSRB_temp;
	
// ACSR – Analog Comparator Control and Status Register
	// bit 7 ACD Analogue Comparator Disable - 0 = Enable, 1 = Disable
	// bit 6 ACBG Analogue Comparator Bandgap Select
	// 1 = use fixed reference voltage instead of the AIN0 positive input voltage
	// bit 5 ACO Analogue Comparator Output (readable)
	// bit 4 ACI Analogue Comparator Interrupt Flag (readable).
	// bit 3 ACIE Analogue Comparator Interrupt Enable
	// 1 = Enable the Analogue Comparator to generate an Interrupt
	// bit 2 ACIC Analogue Comparator Input Capture Enable
	// 1 Enables Timer/Counter1 Input Capture to be triggered by the Analogue Comparator
	// bit 1, 0 ACIS1, ACIS0 Analogue Comparator Interrupt Mode Select
	// 00 = Comparator interrupt on Output Toggle
	// 01 = Reserved
	// 10 = Comparator interrupt on Falling Output Edge
	// 11 = Comparator interrupt on Rising Output Edge
	ACSR = 0b00001010; // Enable, No bandgap, Interrupt enable, No capture, Interrupt on Falling Output Edge

// DIDR1 – Digital Input Disable Register 1
	// Bit 1, 0 – AIN1D, AIN0D: AIN1, AIN0 Digital Input Disable
	DIDR1 = 0b00000011;
}

ISR(ANALOG_COMP_vect)	// Analogue Comparator Interrupt Handler
{
	PORTB = 0b00111111; // Briefly Flash LEDs 6 and 7 to indicate interrupt occurred
	_delay_ms(50);
}
