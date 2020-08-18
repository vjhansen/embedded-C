// *******************************************************************************
// Project 		Timer Demo5 Uses all three timers to flash three LEDs at different speeds (Embedded C)
// Target 		ATMEL ATmega1281 micro-controller on STK300 board
// Program		TimerDemo5_3Timers_FlashSeparateLEDs_1281_C.c
// Author		Richard Anthony
// Date			15th September 2013

// Function			Demonstrates how to use the programmable timers
//					Three timers are used (0,1,3), each one flashes a single LED at a specific rate


// I/O hardware		Uses on-board LEDs on Port B (output)
// *****************************************

#include <avr/io.h>
#include <avr/interrupt.h>

// Declare functions (these will be in a separate header file in larger programs)
void InitialiseGeneral();
void InitialiseTimer0();
void InitialiseTimer1();
void InitialiseTimer3();
void Toggle_LED0();
void Toggle_LED3();
void Toggle_LED6();

int main( void )
{
	InitialiseGeneral();

	InitialiseTimer0();
	InitialiseTimer1();
	InitialiseTimer3();
	
	while(1)        	// Loop
	{
		// In this example all the application logic is within the Timer1 interrupt handler
	}
}

void InitialiseGeneral()
{
	DDRB = 0xFF;			// Configure PortB direction for Output
	PORTB = 0xFF;			// Set all LEDs initially off (inverted on the board, so '1' = off)
	
	sei();
}

void InitialiseTimer0()		// Configure to generate an interrupt after a 1/4 Second interval
{
	TCCR0A = 0b00000000;	// Normal port operation (OC0A, OC0B), Normal waveform generation
	TCCR0B = 0b00000101;	// Normal waveform generation, Use 1024 prescaler
	// For 8 MHz clock (with 1024 prescaler) Overflow occurs after counting to 256
	// (but already divided by 1024)
	// So overflow occurs after 1024 * 256 / 8000000 = 0.033 seconds
	// So 8 bar movements (full range) will happen in 1/4 second

	// For 1 MHz clock (with 1024 prescaler) Overflow occurs after counting to 256
	// (but already divided by 1024)
	// So overflow occurs after 1024 * 256 / 1000000 = 0.26 seconds
	TCNT0 = 0b00000000;	// Timer/Counter count/value register

	TIMSK0 = 0b00000001;		// Use 'Overflow' Interrupt, i.e. generate an interrupt
								// when the timer reaches its maximum count value
}

ISR(TIMER0_OVF_vect) // TIMER0_Overflow_Handler (Interrupt Handler for Timer 0)
{
	Toggle_LED0();		// Bit 0 is toggled every time the interrupt occurs
}

void InitialiseTimer1()		// Configure to generate an interrupt after a 2-Second interval
{
	TCCR1A = 0b00000000;	// Normal port operation (OC1A, OC1B, OC1C), Clear Timer on 'Compare Match' (CTC) waveform mode)
	TCCR1B = 0b00001101;	// CTC waveform mode, use prescaler 1024
	TCCR1C = 0b00000000;
	
	// For 1 MHz clock (with 1024 prescaler) to achieve a 2 second interval:
	// Need to count 2 million clock cycles (but already divided by 1024)
	// So actually need to count to (2000000 / 1024 =) 1953 decimal, = 7A1 Hex
	OCR1AH = 0x07; // Output Compare Registers (16 bit) OCR1BH and OCR1BL
	OCR1AL = 0xA1;

	TCNT1H = 0b00000000;	// Timer/Counter count/value registers (16 bit) TCNT1H and TCNT1L
	TCNT1L = 0b00000000;
	TIMSK1 = 0b00000010;	// bit 1 OCIE1A		Use 'Output Compare A Match' Interrupt, i.e. generate an interrupt
							// when the timer reaches the set value (in the OCR1A registers)
}

ISR(TIMER1_COMPA_vect) // TIMER1_CompareA_Handler (Interrupt Handler for Timer 1)
{
	Toggle_LED3();		// Bit 3 is toggled every time the interrupt occurs
}

void InitialiseTimer3()		// Configure to generate an interrupt after a 0.5 Second interval
{
	TCCR3A = 0b00000000;	// Normal port operation (OC1A, OC1B, OC1C), Clear Timer on 'Compare Match' (CTC) waveform mode)
	TCCR3B = 0b00001101;	// CTC waveform mode, use prescaler 1024
	TCCR3C = 0b00000000;
	
	// For 1 MHz clock (with 1024 prescaler) to achieve a 0.5 second interval:
	// Need to count 500,000 clock cycles (but already divided by 1024)
	// So actually need to count to (500000 / 1024 =) 488 decimal, = 1E8 Hex
	OCR3AH = 0x01; // Output Compare Registers (16 bit) OCR1BH and OCR1BL
	OCR3AL = 0xE8;

	TCNT3H = 0b00000000;	// Timer/Counter count/value registers (16 bit) TCNT3H and TCNT3L
	TCNT3L = 0b00000000;
	TIMSK3 = 0b00000010;	// bit 1 OCIE3A		Use 'Output Compare A Match' Interrupt, i.e. generate an interrupt
							// when the timer reaches the set value (in the OCR3A registers)
}

ISR(TIMER3_COMPA_vect) // TIMER3_Overflow_Handler (Interrupt Handler for Timer 3)
{
	Toggle_LED6();		// Bit 6 is toggled every time the interrupt occurs
}

void Toggle_LED0()	// Swap the value of port B bit 0 (if 0, set to 1; if 1 set to 0)
{
	unsigned char temp;		// Local variable (private to this function)
	temp = PINB;			// (Can actually read the port value even when its set as output)
	if(temp & 0b00000001)	// Bitwise version of AND - Check if bit 0 is currently set '1'
	{
		temp &= 0b11111110;	// Currently set, so force it to '0' without changing other bits
	}
	else
	{
		temp |= 0b00000001;	// Currently cleared, so force it to '1' without changing other bits
	}
	PORTB = temp;
}

void Toggle_LED3()	// Swap the value of port B bit 3 (if 0, set to 1; if 1 set to 0)
{
	unsigned char temp;		// Local variable (private to this function)
	temp = PINB;			// (Can actually read the port value even when its set as output)
	if(temp & 0b00001000)	// Check if bit 3 is currently set '1'
	{
		temp &= 0b11110111;	// Currently set, so force it to '0' without changing other bits
	}
	else
	{
		temp |= 0b00001000;	// Currently cleared, so force it to '1' without changing other bits
	}
	PORTB = temp;
}

void Toggle_LED6()	// Swap the value of port B bit 6 (if 0, set to 1; if 1 set to 0)
{
	unsigned char temp;		// Local variable (private to this function)
	temp = PINB;			// (Can actually read the port value even when its set as output)
	if(temp & 0b01000000)	// Check if bit 6 is currently set '1'
	{
		temp &= 0b10111111;	// Currently set, so force it to '0' without changing other bits
	}
	else
	{
		temp |= 0b01000000;	// Currently cleared, so force it to '1' without changing other bits
	}
	PORTB = temp;
}