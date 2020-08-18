// *****************************************
// Project 		Timer Demo #4 Use a Timer to measure the time interval between events
// Target 		ATmega1281 on STK300
// Program		TimerDem4_MeasuringTimeBetweenEvents_1281_C.c
// Author		Richard Anthony
// Date			15th September 2013
//
// Function		Demonstrates how to use a programmable timer to measure a time interval
//					In this demonstration the timer is initially 'configured' but is not 'started'.
//					The first 'event' causes the timer to be started
//					The second 'event' causes the timer to be stopped and the elapsed time
//					between the two events is displayed on the on-board LEDs.
//
//				Demonstrates Hardware Interrupts
//                 The first 'event' that starts the timer is Hardware Interrupt INT0
//					(alternate function of port D bit 0). By connecting the switches of
//					the external Switches and Lights box to port D we can use the bit 0
//					switch to generate an interrupt on pin INT0.

//                 The second 'event' that stops the timer is Hardware Interrupt INT1
//					(alternate function of port D bit 1). By connecting the switches of
//					the external Switches and Lights box to port D we can use the bit 1
//					switch to generate an interrupt on pin INT1.

//	Timer 'range'	Uses 16-bit Counter/Timer 1 (CT1)
//					This timer can measure a time period of up to 65535 clock pulses
//					(i.e.2 to the power of 16, minus 1)
//					(This is the biggest number we can load into the timer's register)

//					If the system clock is used directly
//						If the clock runs at 1Mhz, the maximum time period is
//						65535 / 1000000 = 65.5 Milliseconds
//						If the clock runs at 8Mhz, the maximum time period is
//						65535 / 8000000 = 8.2 Milliseconds

//	Prescaler		The prescaler options for timer 1 are:
//					1 (no prescaling), 8, 64, 256 or 1024

//					If the 1024 prescaler is used
//						If the clock runs at 1Mhz, the maximum time period is
//						(65535 * 1024) / 1000000 = 67 Seconds
//						If the clock runs at 8Mhz, the maximum time period is
//						(65535 * 1024) / 8000000 = 8.4 Seconds

// 					If the time period between events exceeds the maximum timeable period,
//					the Timer Overflow Interrupt will be triggered and will flash the LEDs
//					to signal that the timer has overflowed

// I/O hardware	Uses on-board LEDs on Port B to display the time interval, in seconds
//					Uses External Switches on Port D to provide the start / stop triggers

//					While counting, displays 'live' the number of elapsed seconds between the start and stop events
//					After stopped, displays the 'frozen' number of elapsed seconds between the start and stop events
//
// *****************************************

#include <avr/io.h>
#include <avr/interrupt.h>

// Declare functions (these will be in a separate header file in larger programs)
void InitialiseGeneral();
void Initialise_HW_Interrupts();
void InitialiseTimer1();
void DisplayLED();

// Declare global variables
unsigned char ElapsedSeconds_Count;	// An 'unsigned char' is an 8-bit numeric value, i.e. a single byte

int main( void )
{
	ElapsedSeconds_Count = 0x00;		// Initialise the Elapsed Time counter
	Initialise_HW_Interrupts();
	InitialiseTimer1();
	InitialiseGeneral();
	
	while(1)        		// Loop
	{
		// In this example all the application logic is within the Timer1 interrupt handler
	}
}

void InitialiseGeneral()
{
	DDRB = 0xFF;			// Configure PortB direction for Output
	PORTB = 0xFF;			// Set all LEDs initially off (inverted on the board, so '1' = off)
	DDRD = 0x00;			// Configure PortD direction for Input
	
	sei();
}

void Initialise_HW_Interrupts()
{
	EICRA = 0b00001010;		// INT 3,2 not used, Interrupt Sense (INT1, INT0) falling-edge triggered
	EICRB = 0b00000000;		// INT7 ... 4 not used
	
	EIMSK = 0b00000011;		// Enable INT1, INT0
	EIFR = 0b00000011;		// Clear INT1 and INT0 interrupt flags (in case a spurious interrupt has occurred during chip startup)
}

ISR(INT0_vect) // Hardware_INT0_Handler (Interrupt Handler for INT0)
{
	ElapsedSeconds_Count = 0;	// Clear the elapsed time counter
	DisplayLED();				// Clear the display
	TCCR1B = 0b00001101;		// Start the timer (CTC and Prescaler 1024)
}

ISR(INT1_vect) // Hardware_INT1_Handler (Interrupt Handler for INT1)
{
	TCCR1B = 0b00001000;	// Stop the timer (CTC, no clock)
	DisplayLED();			// Display the amount of time elapsed
}

void InitialiseTimer1()		// Configure to generate an interrupt after a 1-Second interval
{
	TCCR1A = 0b00000000;	// Normal port operation (OC1A, OC1B, OC1C), Clear Timer on 'Compare Match' (CTC) waveform mode)
	TCCR1B = 0b00001000;	// CTC waveform mode, initially stopped (no clock)
	TCCR1C = 0b00000000;
	
	// For 1 MHz clock (with 1024 prescaler) to achieve a 1 second interval:
	// Need to count 1 million clock cycles (but already divided by 1024)
	// So actually need to count to (1000000 / 1024 =) 976 decimal, = 3D0 Hex
	OCR1AH = 0x03; // Output Compare Registers (16 bit) OCR1BH and OCR1BL
	OCR1AL = 0xD0;

	TCNT1H = 0b00000000;	// Timer/Counter count/value registers (16 bit) TCNT1H and TCNT1L
	TCNT1L = 0b00000000;
	TIMSK1 = 0b00000010;	// bit 1 OCIE1A		Use 'Output Compare A Match' Interrupt, i.e. generate an interrupt
							// when the timer reaches the set value (in the OCR1A register)
}

ISR(TIMER1_COMPA_vect) // TIMER1_CompareA_Handler (Interrupt Handler for Timer 1)
{
	ElapsedSeconds_Count++; // Increment the number of elapsed seconds while the timer has been running
	DisplayLED();			// Display the amount of time elapsed
}

void DisplayLED()
{
	unsigned char Output_LED_value;
	Output_LED_value = ~ElapsedSeconds_Count;   // Take one's complement, because on-board LEDs are inverted (i.e. 0 = ON, 1 = OFF)
	PORTB = Output_LED_value;
}
