 //*****************************************
// Project 		Vibration Sensor Demonstration (with simple signal integration) (Embedded C)
// Target 		ATmega1281 on STK300
// Program		Vibration_INT7_Integrated_1281.c
// Author		Richard Anthony
// Date			26th September 2015

// Function		The Vibration Sensor is a sensitive motion-triggered switch.
//				The sensor is connected to INT7 (Port E bit 7).
//				Interrupts are generated each time the sensor moves.
//				The sensor is so sensitive that many interrupts can be generated for a single movement.

//				Each time an interrupt occurs, a count is incremented.

//				However, the pure count of interrupts is not particularly easy to interpret, in terms of the AMOUNT of vibration
//				This program uses a simple integration technique to represent the Vibration Strength in terms of the 
//				number of interrupts per unit time.

//				The Vibration Strength value is displayed on the On-Board LEDs.

//				Port B is set for output (on-board LEDs)
//*****************************************

#include <avr/io.h>
#include <avr/interrupt.h>
#include <math.h>	// pow()

void InitialiseGeneral(void);
void Initialise_INT7(void);
void ENABLE_INT7(void);
void DISABLE_INT7(void);
void InitialiseTimer2_4Hz();

volatile unsigned char cVibrationInterruptCount;	// The pure count of interrupts
volatile int iVibrationInterruptCount_Mean;			// The smoothed mean count of interrupts

int main(void)
{
	Initialise_INT7();
	InitialiseGeneral();
	cVibrationInterruptCount = 0;
	PORTB = ~cVibrationInterruptCount;  // LEDS off initially
	iVibrationInterruptCount_Mean = 0;
	InitialiseTimer2_4Hz();
	ENABLE_INT7();

	while(1)
	{
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
	// Set upper limit of count in any one time period to 255
	if(255 > cVibrationInterruptCount)
	{
		cVibrationInterruptCount++;
	}
}

void InitialiseTimer2_4Hz()		// Configure to generate interrupts at a frequency of 4Hz
{	// Note Timer2 is an 8-bit timer
	// With system clock 1MHz, need to count 250,000 clock cycles. Use prescaler 1024, and count to 244 (1024 * 244 = approx 250,000)
	TCCR2A = 0b00000010;	// Timer/Counter Control Register A (Normal port operation, Clear Timer on 'Compare Match' (CTC))
	TCCR2B = 0b000001111;	// Timer/Counter Control Register B (CTC using OCR2A, prescaler 1024)
	OCR2A = 244;			// Output Compare Register A
	TCNT2 = 0b00000000;		// Timer/Counter Register
	TIMSK2 = 0b00000010;	// Timer/Counter2 Interrupt Mask Register (Output Compare A Match Interrupt)
}

ISR(TIMER2_COMPA_vect) // TIMER1_CompareA_Handler (Interrupt Handler for Timer 1)
{
	DISABLE_INT7();		// Disable INT7 to prevent inconsistent / unstable behaviour

	// Compute a mean value using the 'Exponential Smoothing' technique with Alpha = 0.2
	// (this smooths out raw event counts which can be quite volatile) 
	iVibrationInterruptCount_Mean = ((iVibrationInterruptCount_Mean * 8) + ((int)cVibrationInterruptCount * 2)) /10;	// Update the mean value
	cVibrationInterruptCount = 0;	// Reset the raw event count
	
	//	Display the smoothed mean number of pulses that occur in each time quantum in the form of a bar display.
	unsigned char cDisplayvalue = 0;
	if(0 == iVibrationInterruptCount_Mean)
	{
		cDisplayvalue = 0b00000000;
	}

	if(1 <= iVibrationInterruptCount_Mean && 31 >= iVibrationInterruptCount_Mean)
	{
		cDisplayvalue = 0b00000001;
	}
	
	if(32 <= iVibrationInterruptCount_Mean && 63 >= iVibrationInterruptCount_Mean)
	{
		cDisplayvalue = 0b00000010;
	}
	
	if(64 <= iVibrationInterruptCount_Mean && 95 >= iVibrationInterruptCount_Mean)
	{
		cDisplayvalue = 0b00000100;
	}

	if(96 <= iVibrationInterruptCount_Mean && 127 >= iVibrationInterruptCount_Mean)
	{
		cDisplayvalue = 0b00001000;
	}
	
	if(128 <= iVibrationInterruptCount_Mean && 159 >= iVibrationInterruptCount_Mean)
	{
		cDisplayvalue = 0b00010000;
	}

	if(160 <= iVibrationInterruptCount_Mean && 191 >= iVibrationInterruptCount_Mean)
	{
		cDisplayvalue = 0b00100000;
	}

	if(192 <= iVibrationInterruptCount_Mean && 223 >= iVibrationInterruptCount_Mean)
	{
		cDisplayvalue = 0b01000000;
	}

	if(224 <= iVibrationInterruptCount_Mean)
	{
		cDisplayvalue = 0b10000000;
	}

	PORTB = ~cDisplayvalue;		// Display onto the LEDs
	ENABLE_INT7();
}
