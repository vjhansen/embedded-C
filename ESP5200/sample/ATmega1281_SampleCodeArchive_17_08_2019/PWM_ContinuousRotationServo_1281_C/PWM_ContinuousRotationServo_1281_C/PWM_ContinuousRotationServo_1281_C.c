//*****************************************
// Project 		Pulse-Width-Modulation Demonstration (for 'Continuous Rotation' Servo SM-S3317SR and similar) (C)
// Target 		ATmega1281 on STK300
// Program		PWM_ContinuousRotationServo_1281_C.c
// Author		Richard Anthony
// Date			6th November 2015

// Servo types:
//	-> Limited rotation servos have a typical rotation angle of about 180 degrees. 
//		For this type of servo a PWM pulse is used to control the rotation POSITION.

//	-> Continuous rotation servos can rotate through the entire 360 degrees, continuously, these provide rotational moverment rather than precise position control.
//		For this type of servo a PWM pulse is used to control the rotation SPEED.

//		Servo pulse widths and their effects:
//			1300us	= Maximum speed anti-clockwise
//			1500us	= Stopped
//			1700us	= Maximum speed clockwise
//			The Servo pulse should be repeated every 3ms

//			Total count range needed for 3mS pulse interval is 3000 (I)
//			Pulse width (P) ranges from 1300uS to 1700uS
//			The timer count starts at 0 and counts up:
//				For channel A (output pin OC3A is high), until it matches the OCR3A value
//				(OCR3A must hold the pulse-width value P, in uS)
//				OC3A is then switched low, count continues up to TOP
//			ICR1 is used to set the TOP value (i.e. I)
//			_______________                                           _______
//			|              |__________________________________________|

//			Pulse (P)	   P	   Interval (I) includes P but       Cycle ends
//			start       ends       continues after P ends            next cycle begins


// Function		Controls the rotation speed and direction of a 'Continuous Rotation' servo (such as SM-S3317SR), using PWM

//				PWM signal
//					Timer3 OC3A, Signal appears on PORT E bit 3
//					Switches 0 - 7 select preset servo rotation speed and direction settings

//				Fuse settings - 'Device Clock Select' = Calibrated Internal RC Oscillator 1.0 MHz

//	Input / Output
//					PORT B user to display PWM values on LEDs for diagnostic (set for output)
//					PORT D Switches are used to control servo rotation speed and direction settings (set for input)

//					Timer3 OC3A (Output Compare match Output 3A) Port E bit 3
//					Port E bit 3 DDE3 must be configured as an output

//					Timer3 Input Capture Pin (ICP3) Port E bit 7 must be configured
//					as an output to prevent random / spurious values entering the ICR3 register

//					The servo needs external power supply since they can draw up to 550mA
//					which is more than the STK300 board can source.
//					Under 'no load' conditions the SM-S3317SR servo requires 220mA at 5 Volts
//					If the servo is powered directly from an STK300 board there is a chance that
//					the Atmel microcontroller will continually reset as the power supply is brought
//					down below the operating voltage of the Atmel by the servo trying to draw more
//					current than can be sourced.

// Servo 'stopped' setting calibration:
//		The SM-S3317SR servo has a small variable resistor accessible in the center of the underside of the unit.
//		This can be adjusted with a very small screwdriver (and great care) until the servo stops rotating when a 1500uS pulse is applied.
//		Once calibrated, a pulse width lower than 1500uS will cause the servo to rotate in one direction, and a pulse width greater than
//		1500uS will cause the servo to rotate in the other direction.
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
void InitialiseTimer3_FastPWM_ChannelA(void);

int main( void )
{
	InitialiseGeneral();
	InitialiseTimer3_FastPWM_ChannelA();
	
	unsigned char SwitchesValue;
	
	while(1)
	{
		SwitchesValue = ~PIND;	// Read value on switches

		switch(SwitchesValue)
		{
			case Switch_0_Pressed:
				// Pulse width ranges from 1300uS to 1700uS
				OCR3A = 1300;	// Maximum speed anticlockwise
				PORTB = ~0b00000111;
				_delay_ms(80);	// Switch debounce delay
				break;
			case Switch_1_Pressed:
				OCR3A = 1350;	// Anticlockwise
				PORTB = ~0b00000011;
				_delay_ms(80);	// Switch debounce delay
				break;
			case Switch_2_Pressed:
				OCR3A = 1400;	// Anticlockwise
				PORTB = ~0b00000001;
				_delay_ms(80);	// Switch debounce delay
				break;
			case Switch_3_Pressed:
				OCR3A = 1500;	// Stopped (see notes above concerning calibration of the servo)
				PORTB = ~0b00011000;
				_delay_ms(80);	// Switch debounce delay
				break;
			case Switch_4_Pressed:
				OCR3A = 1600;	// Clockwise
				PORTB = ~0b10000000;
				_delay_ms(80);	// Switch debounce delay
				break;
			case Switch_5_Pressed:
				OCR3A = 1633;	// Clockwise
				PORTB = ~0b11000000;
				_delay_ms(80);	// Switch debounce delay
				break;
			case Switch_6_Pressed:
				OCR3A = 1667;	// Clockwise
				PORTB = ~0b11100000;
				_delay_ms(80);	// Switch debounce delay
				break;
			case Switch_7_Pressed:
				OCR3A = 1700;	// Maximum speed clockwise
				PORTB = ~0b11110000;
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
			
	DDRE = 0xFF;	// Port E bit 3 must be set as OUTPUT to provide the PWM pulse on OC3A
					// Port E bit 7 Input Capture 3 Pin (ICP3) must be set as OUTPUT to prevent 
					// random / noise values entering ICR3 (ICR3 used as TOP value for PWM counter)
	PORTE = 0x00;	// Set all bits initially
		
	sei();			// Enable interrupts at global level set Global Interrupt Enable (I) bit
}

void InitialiseTimer3_FastPWM_ChannelA()
{
	// TCCR3A – Timer/Counter 3 Control Register A
	// Bit 7:6 – COMnA1:0: Compare Output Mode for Channel A (For FAST PWM 10 = Clear OC3A on Compare match (Non-Inverting))
	// Bit 5:4 – COMnB1:0: Compare Output Mode for Channel B (For FAST PWM 10 = Clear OC3B on Compare match (Non-Inverting))
	// Bit 3:2 – COMnC1:0: Compare Output Mode for Channel C (For FAST PWM 10 = Clear OC3C on Compare match (Non-Inverting))
	// Bit 1:0 – WGMn1:0: Waveform Generation Mode (Waveform bits WGM3(3..0) 1110 Fast PWM ICR3 is TOP)
	TCCR3A = 0b10000010;	// Fast PWM non inverting, ICR3 used as TOP
	
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
	// For the continuous rotation, the pulses should occur every 3ms, i.e. 3000uS (gives a square wave of 50% duty cycle when at the center of the PWM range)
	// With a 1MHz clock speed, each clock pulse takes 1us, therefore need to count 3000 clock pulses
	ICR3 = 3000;

	// Set Timer/Counter count/value registers (16 bit) TCNT1H and TCNT1L
	TCNT3H = 0; // 16-bit access (write high byte first, read low byte first)
	TCNT3L = 0;

	// Initialise Channel A servo to Stopped (the mid-range value 1500 in the range 1300 to 1700)
	// Set Timer/Counter Output Compare Registers (16 bit) OCR3AH and OCR3AL
	// Pulse width ranges from 1300uS to 1700uS
	// 'Stopped' (Mid range) pulse width 1.5mS = 1500uS pulse width
	OCR3A = 1500;
	
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