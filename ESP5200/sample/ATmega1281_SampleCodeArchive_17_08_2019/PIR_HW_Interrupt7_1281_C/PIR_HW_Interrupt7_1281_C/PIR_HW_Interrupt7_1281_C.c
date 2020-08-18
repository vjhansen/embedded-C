//*****************************************
// Project 		PIR detector example - using Hardware Interrupt 7   PORT E (C)
// Target 		ATmega1281 on STK300
// Program		PIR_HW_Interrupt7_1281_C.c
// Author		Richard Anthony
// Date			7th November 2013
//
// Function		Demonstrates use of Passive InfraRed detector (PIR) using Hardware interrupt 7 (PORT E bit 7)
//				The PIR generates a rising edge when it detects movement (high output = detect mode)
//				The hardware interrupt is therefore configured to trigger on a rising edge

//				**********************************************************************************
//				NOTE: THIS SENSOR HAS A 50-SECOND WARM-UP TIME DURING WHICH THE OUTPUT IS UNSTABLE
//				**********************************************************************************

//				Displays a count of interrupt events on the LEDs

// Ports		Port B set for output (LEDs)
//				Port E bit 7 set for input (needed for the H/W interrupt to operate correctly)
//******************************************************************
#include <avr/io.h>
#include <avr/interrupt.h>

void InitialiseGeneral(void);
void Initialise_INT7(void);
void ENABLE_INT7(void);
void DISABLE_INT7(void);

unsigned char uEventCount;

int main(void)
{
	uEventCount = 0;
	Initialise_INT7();
	InitialiseGeneral();
	ENABLE_INT7();

    while(1)
    {
		PORTB = ~uEventCount;
	}
}

void InitialiseGeneral()
{
	DDRB = 0xFF;			// Configure PortB direction for Output
	PORTB = 0xFF;			// Set all LEDs initially off (inverted on the board, so '1' = off)

	DDRE = 0b01111111;		// Port E Data Direction Register (Port E bit 7 set for input)
	PORTE = 0b00000000;		// Bits 0-6 form a groundplane, bit 7 pullup resistor NOT set

	sei();					// Enable interrupts at global level, set Global Interrupt Enable (I) bit
}

void Initialise_INT7()
{
	// EICRA – External Interrupt Control Register A
	// Bits 7:0 – ISC31, ISC30 – ISC00, ISC00: External Interrupt 3 - 0 Sense Control Bits
	// 00 The low level of INTn generates an interrupt request
	// 01 Any edge of INTn generates asynchronously an interrupt request
	// 10 The falling edge of INTn generates asynchronously an interrupt request
	// 11 The rising edge of INTn generates asynchronously an interrupt request
	EICRA = 0b00000000;

	// EICRB – External Interrupt Control Register B
	// Bits 7:0 – ISC71, ISC70 - ISC41, ISC40: External Interrupt 7 - 4 Sense Control Bits
	// 00 The low level of INTn generates an interrupt request
	// 01 Any edge of INTn generates asynchronously an interrupt request
	// 10 The falling edge of INTn generates asynchronously an interrupt request
	// 11 The rising edge of INTn generates asynchronously an interrupt request
	EICRB = 0b11000000;		// Interrupt Sense (INT7) Rising-edge triggered

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
	uEventCount++;
}

