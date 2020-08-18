//*****************************************
// Project 		Pulse-Width-Modulation Demonstration (LED brightness control) (C)
// Target 		ATmega1281 on STK300
// Program		PWM_LED_Dimmer_1281_C.c
// Author		Richard Anthony
// Date			19th October 2013 (original ATmega8535 version 8th November 2011)

// Function		Uses 16-bit Counter/Timer 5
//				This example uses the PWM output by using both interrupt handlers to signal the pulse changeover points
//				and thus DOES NOT REQUIRE A SIGNAL ON AN OUTPUT PIN - for this reason Timer 5 is used, which does not 
//				have on OC pin on the 64-pin package of the ATmega1281.

//				The PWM signal is interpreted in software using two interrupt handlers:
//					ISR(TIMER5_OVF_vect), the TIMER5_Overflow_Handler (signals Start of new cycle), turns LEDS on
//					ISR(TIMER5_COMPA_vect) the TIMER5_CompareA_Handler (signals the cutoff for the high part of the cycle), turn LEDS off
//				The Pulse-Width-Modulated signal will not appear on any pin directly (Timer 5 does not have any OC pins)

//	Input / Output
//					Switched connected to PORT D to select PWM configuration
//					LEDs connected to Port B to show effect of PWM (changes LED brightness)
//*****************************************
#include <avr/io.h>
#include <avr/interrupt.h>

#define Switch_0_Pressed 0b00000001
#define Switch_1_Pressed 0b00000010
#define Switch_2_Pressed 0b00000100
#define Switch_3_Pressed 0b00001000
#define Switch_4_Pressed 0b00010000
#define Switch_5_Pressed 0b00100000
#define Switch_6_Pressed 0b01000000
#define Switch_7_Pressed 0b10000000

void InitialiseGeneral();
void InitialiseTimer5_PWM_ChannelA();

int main( void )
{
	unsigned char SwitchesValue;
	InitialiseTimer5_PWM_ChannelA();
	InitialiseGeneral(); // Port B must be set for output AFTER the timer setup of OC0
	
    while(1)
    {
		SwitchesValue = ~PIND;	// Read value on switches

		// Set the timer count register depending on which switch was pressed
		// (i.e. set the PWM pulse width, and thus adjust the LED brightness)
		switch(SwitchesValue)
		{
			case Switch_0_Pressed:	// Switch 0 pressed
				OCR5AL = 1;			// 0.5 % duty cycle
				break;
			case Switch_1_Pressed:	// Switch 1 pressed
				OCR5AL = 5;			// 1.9% duty cycle
				break;
			case Switch_2_Pressed:	// Switch 2 pressed
				OCR5AL = 10;		// 3.9% duty cycle
				break;
			case Switch_3_Pressed:	// Switch 3 pressed
				OCR5AL = 16;		// 6.25% duty cycle
				break;
			case Switch_4_Pressed:	// Switch 4 pressed
				OCR5AL = 24;		// 9.3% duty cycle
				break;
			case Switch_5_Pressed:	// Switch 5 pressed
				OCR5AL = 32;		// 12.5% duty cycle 
				break;
			case Switch_6_Pressed:	// Switch 6 pressed
				OCR5AL = 64;		// 25% duty cycle
				break;
			case Switch_7_Pressed:	// Switch 7 pressed
				OCR5AL = 96;		// 37.5% duty cycle (LEDS appear near-full brightness)
				break;
		}
	}
}

void InitialiseGeneral()
{
	DDRB = 0xFF;			// Set Port B as output (LEDs)
	PORTB = 0xFF;			// LEDs off initially
	DDRD = 0x00;			// Set Port D as input (On-board switches to control pulse width)

	sei();					// Enable interrupts at global level set Global Interrupt Enable (I) bit
}

void InitialiseTimer5_PWM_ChannelA()
{
	// TCCR5A – Timer/Counter 5 Control Register A
	// Bit 7:6 – COMnA1:0: Compare Output Mode for Channel A
	// Bit 5:4 – COMnB1:0: Compare Output Mode for Channel B
	// Bit 3:2 – COMnC1:0: Compare Output Mode for Channel C
	// Bit 1:0 – WGMn1:0: Waveform Generation Mode (0101 Fast PWM, 8-bit)
	TCCR5A = 0b00000001;	// No output pins in use, set all to normal mode, waveform  = Fast PWM, 8-bit
	
	// TCCR5B – Timer/Counter 5 Control Register B
	// Bit 7 – ICNCn: Input Capture Noise Canceler
	// Bit 6 – ICESn: Input Capture Edge Select
	// Bit 5 – Reserved Bit
	// Bit 4:3 – WGMn3:2: Waveform Generation Mode (0101 Fast PWM, 8-bit)
	// Bit 2:0 – CSn2:0: Clock Select (010 = 8 prescaler)
	TCCR5B = 0b00001010; // waveform  = Fast PWM, 8-bit, 8 prescaler
	
	// TCCR5C – Timer/Counter 5 Control Register C
	// Bit 7 – FOCnA: Force Output Compare for Channel A
	// Bit 6 – FOCnB: Force Output Compare for Channel B
	// Bit 5 – FOCnC: Force Output Compare for Channel C
	TCCR5C = 0b00000000;
	
	// TCNT5H and TCNT5L –Timer/Counter 5
	TCNT5H = 0;
	TCNT5L = 0;
	
	// OCR5AH and OCR5AL – Output Compare Register 5 A
	OCR5AH = 0;
	OCR5AL = 24;

	// TIMSK5 – Timer/Counter 5 Interrupt Mask Register
	// Bit 5 – ICIEn: Timer/Countern, Input Capture Interrupt Enable
	// Bit 3 – OCIEnC: Timer/Countern, Output Compare C Match Interrupt Enable
	// Bit 2 – OCIEnB: Timer/Countern, Output Compare B Match Interrupt Enable
	// Bit 1 – OCIEnA: Timer/Countern, Output Compare A Match Interrupt Enable
	// Bit 0 – TOIEn: Timer/Countern, Overflow Interrupt Enable
	TIMSK5 = 0b00000011; // Enable OCIE5A and TOIE5 interrupts

	// TIFR5 – Timer/Counter5 Interrupt Flag Register
	TIFR5 = 0b00101111;		// Clear all flags
}

ISR(TIMER5_OVF_vect) // TIMER5_Overflow_Handler
{	// Here at Start of new cycle
	PORTB = 0x00; //LEDS on
}	

ISR(TIMER5_COMPA_vect) // TIMER5_CompareA_Handler
{	// Here when reach the cutoff for the high part of the cycle
	PORTB = 0xFF; //LEDS off
}