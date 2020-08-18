//*****************************************
// Project 		Vibration Sensor Demonstration (using Interrupt INT7) (Embedded C)
// Target 		ATmega1281 on STK300
// Program		Vibration_INT7_1281.c
// Author		Richard Anthony
// Date			26th September 2015

// Function		The Vibration Sensor is a sensitive motion-triggered switch.
//				The sensor is connected to INT7 (Port E bit 7).
//				Interrupts are generated each time the sensor moves.
//				The sensor is so sensitive that many interrupts can be generated for a single movement.

//				Each time an interrupt occurs, a count is incremented.
//				The count is displayed on the On-Board LEDs.

//				Port B is set for output (on-board LEDs)
//*****************************************
#include <avr/io.h>
#include <avr/interrupt.h>

void InitialiseGeneral(void);
void Initialise_INT7(void);
void ENABLE_INT7(void);
void DISABLE_INT7(void);

volatile unsigned char cVibrationInterruptCount;

int main(void)
{
	Initialise_INT7();
	InitialiseGeneral();
	cVibrationInterruptCount = 0;
	ENABLE_INT7();

	while(1)
	{
		PORTB = ~cVibrationInterruptCount;
	}
}

void InitialiseGeneral()
{
	DDRB = 0xFF;		// Configure PortB direction for Output
	PORTB = 0xFF;		// Set all LEDs initially off (inverted on the board, so '1' = off)

	DDRE = 0x00;		// Configure PortE bit 7 direction forInput
	PORTE = 0b10000000;	// Bit bit 7 pullup resistor is set (the pin will be at '1' when the vibration switch is open, '0' when closed)

	sei();	// Enable interrupts at global level set Global Interrupt Enable (I) bit
}

void Initialise_INT7()
{
	// EICRA – External Interrupt Control Register A
	// Bits 7:0 – ISC31, ISC30 – ISC00, ISC00: External Interrupt 3 - 0 Sense Control Bits
	// 00 The low level of INTn generates an interrupt request
	// 01 Any edge of INTn generates asynchronously an interrupt request
	// 10 The falling edge of INTn generates asynchronously an interrupt request
	// 11 The rising edge of INTn generates asynchronously an interrupt request
	EICRA = 0b00000000;		// Configures INT3,2,1,0

	// EICRB – External Interrupt Control Register B
	// Bits 7:0 – ISC71, ISC70 - ISC41, ISC40: External Interrupt 7 - 4 Sense Control Bits
	// 00 The low level of INTn generates an interrupt request
	// 01 Any edge of INTn generates asynchronously an interrupt request
	// 10 The falling edge of INTn generates asynchronously an interrupt request
	// 11 The rising edge of INTn generates asynchronously an interrupt request
	EICRB = 0b10000000;		// Interrupt Sense (INT7) Falling-edge triggered

	// EIMSK – External Interrupt Mask Register
	// Bits 7:0 – INT7:0: External Interrupt Request 7 - 0 Enable
	EIMSK = 0b00000000;		// Initially disabled, set bit 7 to Enable H/W Int 7

	// EIFR – External Interrupt Flag Register
	// Bits 7:0 – INTF7:0: External Interrupt Flags 7 - 0
	EIFR = 0b11111111;		// Clear all HW interrupt flags (in case a spurious interrupt has occurred during chip startup)
}

void ENABLE_INT7()
{
	EIMSK = 0b10000000;		// Enable H/W Int 7
}

void DISABLE_INT7()
{
	EIMSK = 0b00000000;		// Disable H/W Int 7
}

ISR(INT7_vect) // Interrupt Handler for H/W INT 7
{
	cVibrationInterruptCount++;
}
