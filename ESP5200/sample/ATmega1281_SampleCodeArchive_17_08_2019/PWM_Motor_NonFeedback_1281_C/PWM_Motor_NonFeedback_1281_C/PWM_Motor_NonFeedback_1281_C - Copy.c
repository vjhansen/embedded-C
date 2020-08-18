//*******************************************************************************
// Project 		PWM - Motor Speed Control Demonstration (Uses simple motor rig - without feedback) (Embedded C)
// Target 		ATMEL ATmega1281 micro-controller on STK300 board
// Program		PWM_Motor_NonFeedback_1281_C.c
// Author		Richard Anthony
// Date			19th October 2013 (original ATmega8535 version 21st October 2011)

// Function		Controls Motor speed using PWM
//					Uses the simple motor rig - i.e. the one NOT in a plastic box.
//					The Simple Motor Rig does not have a feedback mechanism
//					The Simple Motor Rig uses OC3B (Timer 3 PWM channel B, Port E bit 4)
//					NOTE: The Boxed Motor Rig (with feedback) uses OC3C (Timer 3 PWM channel C, Port E bit 5)

//				PWM signal
//					Timer3 OC3B, Signal appears on PORT E bit B
//					Switches 0 - 7 select preset Motor speed (energy) settings

//				Fuse settings - 'Device Clock Select' = Calibrated Internal RC Oscillator 1.0 MHz

//	Input / Output
//					PORT B user to display PWM values on LEDs for diagnostic (set for output)
//					PORT D Switches are used to control Motor speed (energy) settings (set for input)

//					Timer3 OC3B (Output Compare match Output 3B) Port E bit 4
//					Port E bit 4 DDE4 must be configured as an output

//					Timer3 Input Capture Pin (ICP3) Port E bit 7 must be configured 
//					as an output to prevent random / spurious values entering the ICR3 register
//*****************************************

#include <avr/io.h>
#include <avr/interrupt.h>
#include <util/delay.h>		// Needed so the _delay_ms() function can be used

#define Switch_0_Pressed 0b00000001
#define Switch_1_Pressed 0b00000010
#define Switch_2_Pressed 0b00000100
#define Switch_3_Pressed 0b00001000
#define Switch_4_Pressed 0b00010000
#define Switch_5_Pressed 0b00100000
#define Switch_6_Pressed 0b01000000
#define Switch_7_Pressed 0b10000000

void InitialiseGeneral(void);
void InitialiseTimer3_FastPWM_ChannelB(void);

int main( void )
{
	InitialiseGeneral();
	InitialiseTimer3_FastPWM_ChannelB();
	
	unsigned char SwitchesValue;
	
	while(1)
	{
		SwitchesValue = ~PIND;	// Read value on switches

		switch(SwitchesValue)
		{
			case Switch_0_Pressed:
				// Pulse width ranges from 750uS to 2250uS (range is 1500us, 1/7 range (25 degrees) is 214us)
				OCR3B = 0x1000;	// Minimum value
				PORTB = ~0x10;
				_delay_ms(80);	// Switch debounce delay
				break;
			case Switch_1_Pressed:
				OCR3B = 0x1200;
				PORTB = ~0x12;
				_delay_ms(80);	// Switch debounce delay
				break;
			case Switch_2_Pressed:
				OCR3B = 0x1400;
				PORTB = ~0x14;
				_delay_ms(80);	// Switch debounce delay
				break;
			case Switch_3_Pressed:
				OCR3B = 0x1800;
				PORTB = ~0x18;
				_delay_ms(80);	// Switch debounce delay
				break;
			case Switch_4_Pressed:
				OCR3B = 0x1C00;
				PORTB = ~0x1C;
				_delay_ms(80);	// Switch debounce delay
				break;
			case Switch_5_Pressed:
				OCR3B = 0x2000;
				PORTB = ~0x20;
				_delay_ms(80);	// Switch debounce delay
				break;
			case Switch_6_Pressed:
				OCR3B = 0x2400;
				PORTB = ~0x24;
				_delay_ms(80);	// Switch debounce delay
				break;
			case Switch_7_Pressed:
				OCR3B = 0x2800;
				PORTB = ~0x28;
				_delay_ms(80);	// Switch debounce delay
				break;
		}
	}
}

void InitialiseGeneral()
{
	DDRB = 0xFF;	// Port B connected to LEDs for diagnostic output
	PORTB = 0xFF;	// LEDs initially off

	DDRD = 0x00;	// Configure PortD direction for Input (switches)
			
	DDRE = 0xFF;	// Port E bit 4 must be set as OUTPUT to provide the PWM pulse on OC3B
					// Port E bit 7 Input Capture 3 Pin (ICP3) must be set as OUTPUT to prevent 
					// random / noise values entering ICR3 (ICR3 used as TOP value for PWM counter)
	PORTE = 0x00;	// Set all bits initially
		
	sei();			// Enable interrupts at global level set Global Interrupt Enable (I) bit
}

void InitialiseTimer3_FastPWM_ChannelB()
{
// TCCR3A – Timer/Counter 3 Control Register A
	// Bit 7:6 – COMnA1:0: Compare Output Mode for Channel A (For FAST PWM 10 = Clear OC3A on Compare match (Non-Inverting))
	// Bit 5:4 – COMnB1:0: Compare Output Mode for Channel B (For FAST PWM 10 = Clear OC3B on Compare match (Non-Inverting))
	// Bit 3:2 – COMnC1:0: Compare Output Mode for Channel C (For FAST PWM 10 = Clear OC3C on Compare match (Non-Inverting))
	// Bit 1:0 – WGMn1:0: Waveform Generation Mode (Waveform bits WGM3(3..0) 1110 Fast PWM ICR3 is TOP)
	TCCR3A = 0b00100010;	// Fast PWM non inverting, ICR3 used as TOP
	
// TCCR3B – Timer/Counter 3 Control Register B
	// Bit 7 – ICNCn: Input Capture Noise Canceler
	// Bit 6 – ICESn: Input Capture Edge Select
	// Bit 5 – Reserved Bit
	// Bit 4:3 – WGMn3:2: Waveform Generation Mode
	// Bit 2:0 – CSn2:0: Clock Select
	TCCR3B = 0b00011001;	// Fast PWM, Use Prescaler '1'

// TCCR3C – Timer/Counter 3 Control Register C
	// Bit 7 – FOCnA: Force Output Compare for Channel A
	// Bit 6 – FOCnB: Force Output Compare for Channel B
	// Bit 5 – FOCnC: Force Output Compare for Channel C
	TCCR3C = 0b00000000;

// Set Timer/Counter3 Input Capture Register (16 bit) ICR3
	// Can only be written to when using a waveform generation mode that uses ICR3 to define the TOP value
	ICR3H = 0xFF; // 16-bit access (write high byte first, read low byte first)
	ICR3L = 0xFF;

// Set Timer/Counter count/value registers (16 bit) TCNT1H and TCNT1L
	TCNT3H = 0; // 16-bit access (write high byte first, read low byte first)
	TCNT3L = 0;

// Initialise Channel B to motor high-speed setting (to overcome stall speed on program startup)
	// Set Timer/Counter Output Compare Registers (16 bit) OCR3AH and OCR3AL
	OCR3B = 0x2800;
	PORTB = ~0x28;
// TIMSK3 – Timer/Counter 3 Interrupt Mask Register
	// Bit 5 – ICIEn: Timer/Countern, Input Capture Interrupt Enable
	// Bit 3 – OCIEnC: Timer/Countern, Output Compare C Match Interrupt Enable
	// Bit 2 – OCIEnB: Timer/Countern, Output Compare B Match Interrupt Enable
	// Bit 1 – OCIEnA: Timer/Countern, Output Compare A Match Interrupt Enable
	// Bit 0 – TOIEn: Timer/Countern, Overflow Interrupt Enable
	TIMSK3 = 0b00000000;	// No interrupts needed, PWM pulses appear directly on OC3A (Port E Bit3)
	
// TIFR3 – Timer/Counter3 Interrupt Flag Register
	TIFR3 = 0b00101111;		// Clear all interrupt flags
}
