//*****************************************
// Project 		SERVO control (triple) Demonstration using Pulse-Width-Modulation (C)
// Target 		ATmega1281 on STK300
// Program		PWM_Servo_Triple_Switches_1281_C.c
// Author		Richard Anthony
// Date			19th October 2013 (original 9th November 2011)

// Function		Controls position of three SERVOs by changing the pulse width of the control signals
//				Timer 3 is used to produce three independent PWM signals which appear on Port E pins
//				Uses 'FAST PWM' mode of 16-bit Timer3 and the '1' prescaler.

//				Switches are used to move the servos in increments of approximately 15 degrees (full movement is approx 180 degrees)

//				PWM signals
//					PWM #1 Timer3 OC3A, Signal appears on PORT E bit 3
//					Switch 0 causes the servo to rotate Clockwise in increments of approximately 15 degrees
//					Switch 1 causes the servo to rotate Anticlockwise in increments of approximately 15 degrees

//					PWM #2 Timer3 OC3B, Signal appears on PORT E bit 4
//					Switch 3 causes the servo to rotate Clockwise in increments of approximately 15 degrees
//					Switch 4 causes the servo to rotate Anticlockwise in increments of approximately 15 degrees

//					PWM #3 Timer3 OC3C, Signal appears on PORT E bit 5
//					Switch 6 causes the servo to rotate Clockwise in increments of approximately 15 degrees
//					Switch 7 causes the servo to rotate Anticlockwise in increments of approximately 15 degrees


//				Fuse settings - 'Device Clock Select' = Calibrated Internal RC Oscillator 1.0 MHz


//				SERVO characteristics (Typical):
//					Device 'Standard' SERVO - Such as - (WWW.GWS.COM.TW      Part number S03N STD)
//					3-Wire system (no feedback) Red 5V, Black 0V, White or Yellow 3-5V short pulse (see below)
//					Pulse width to control position 0.75ms - 2.25ms ("1.5ms neutral")
//					Pulse rate, approximately every 18ms
//					Torque 3.4Kg / cm
//					Rotation 170 degrees +/- 10 degrees
//					Current draw standby 8mA, maximum 550mA
//					"Can reverse polarity to change direction", - not tested yet (Not needed for Embedded Systems Level 2)

//				The clock source is the on-chip I/O clock. The demo assumes 1MHz clock
//					so timer prescaler not needed (enables setting pulse-width at 1uS accuracy)	
//					Clock prescaler options are 1,8,64,256,1024
//					
//					Total count range needed for 18mS pulse interval is 18000 (I)
//					Pulse width (P) ranges from 750uS to 2250uS
//					Each count starts at 0 and counts up:
//						For channel A (output pin OC3A is high), until it matches the OCR3A value
//						(OCR3A must hold the pulse-width value P, in uS)
//						OC3A is then switched low, count continues up to TOP
//					ICR1 is used to set the TOP value (i.e. I)
//					_______________                                           _______
//					|              |__________________________________________|
//
//					Pulse (P)	   P	   Interval (I) includes P but     Cycle ends
//					start         ends       continues after P ends      next cycle begins

//	Input / Output
//					PORT D Switches are used to control servo rotation (set for input)

//					Timer3 OC3A (Output Compare match Output 3A) Port E bit 3
//					Port E bit 3 DDE3 must be configured as an output
//					Timer3 OC3B (Output Compare match Output 3B) Port E bit 4
//					Port E bit 4 DDE4 must be configured as an output
//					Timer3 OC3C (Output Compare match Output 3C) Port E bit 5
//					Port E bit 5 DDE5 must be configured as an output

//					Timer3 Input Capture Pin (ICP3) Port E bit 7 must be configured 
//					as an output to prevent random / spurious values entering the ICR3 register

//					Servos need external power supply since they can draw up to 550mA
//					which is more than the STK300 board can source.

//*****************************************

#include <avr/io.h>
#include <avr/interrupt.h>
#include <util/delay.h>		// needed so the _delay_ms() function can be used

#define Switch_0_Pressed 0b00000001
#define Switch_1_Pressed 0b00000010

#define Switch_3_Pressed 0b00001000
#define Switch_4_Pressed 0b00010000

#define Switch_6_Pressed 0b01000000
#define Switch_7_Pressed 0b10000000

void InitialiseGeneral(void);
void InitialiseTimer3_FastPWM_Triple(void);

int main( void )
{
	InitialiseGeneral();
	InitialiseTimer3_FastPWM_Triple();
	
	unsigned char SwitchesValue;
	
	while(1)
	{
		SwitchesValue = ~PIND;	// Read value on switches

		switch(SwitchesValue)
		{
			case Switch_0_Pressed:	// Move servo A clockwise approximately 15 degrees
				// Pulse width ranges from 750uS to 2250uS (range is 1500us, 1/12 range (15 degrees) is 125us
				if(OCR3A < 2250)
				{
					OCR3A += 125;
				}
				_delay_ms(80);	// Switch debounce delay
				break;
			case Switch_1_Pressed:	// Move servo A Anti-clockwise approximately 15 degrees
				// Pulse width ranges from 750uS to 2250uS (range is 1500us, 1/12 range (15 degrees) is 125us
				if(OCR3A > 750)
				{
					OCR3A -= 125;
				}
				_delay_ms(80);	// Switch debounce delay
				break;

			case Switch_3_Pressed:	// Move servo B clockwise approximately 15 degrees
				if(OCR3B < 2250)
				{
					OCR3B += 125;
				}
				_delay_ms(80);	// Switch debounce delay
				break;
			case Switch_4_Pressed:	// Move servo B Anti-clockwise approximately 15 degrees
				if(OCR3B > 750)
				{
					OCR3B -= 125;
				}
				_delay_ms(80);	// Switch debounce delay
				break;

			case Switch_6_Pressed:	// Move servo C clockwise approximately 15 degrees
				if(OCR3C < 2250)
				{
					OCR3C += 125;
				}
				_delay_ms(80);	// Switch debounce delay
				break;
			case Switch_7_Pressed:	// Move servo C Anti-clockwise approximately 15 degrees
				if(OCR3C > 750)
				{
					OCR3C -= 125;
				}
				_delay_ms(80);	// Switch debounce delay
				break;
		}
	}
}

void InitialiseGeneral()
{
	DDRD = 0x00;	// Configure PortD direction for Input (switches)
			
	DDRE = 0xFF;	// Port E bit 3 must be set as OUTPUT to provide the PWM pulse on OC3A
					// Port E bit 4 must be set as OUTPUT to provide the PWM pulse on OC3B
					// Port E bit 5 must be set as OUTPUT to provide the PWM pulse on OC3C
					// Port E bit 7 Input Capture 3 Pin (ICP3) must be set as OUTPUT to prevent 
					// random / noise values entering ICR3 (ICR3 used as TOP value for PWM counter)
	PORTE = 0x00;	// Set all bits initially
		
	sei();			// Enable interrupts at global level set Global Interrupt Enable (I) bit
}

void InitialiseTimer3_FastPWM_Triple()
{
// TCCR3A – Timer/Counter 3 Control Register A
	// Bit 7:6 – COMnA1:0: Compare Output Mode for Channel A (For FAST PWM 10 = Clear OC3A on Compare match (Non-Inverting))
	// Bit 5:4 – COMnB1:0: Compare Output Mode for Channel B (For FAST PWM 10 = Clear OC3B on Compare match (Non-Inverting))
	// Bit 3:2 – COMnC1:0: Compare Output Mode for Channel C (For FAST PWM 10 = Clear OC3C on Compare match (Non-Inverting))
	// Bit 1:0 – WGMn1:0: Waveform Generation Mode (Waveform bits WGM3(3..0) 1110 Fast PWM ICR3 is TOP)
	TCCR3A = 0b10101010;	// Fast PWM non inverting, ICR3 used as TOP
	
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
	// For the SERVO, the pulses should occur every 18ms, i.e. 18000uS
	// With a 1MHz clock speed, each clock pulse takes 1us, therefore need to count 18000 clock pulses
	// Decimal 18000 = 0x4650
	// This count value defines where a single cycle ends. 
	// The actual pulse width is much shorter than the whole cycle. 
	ICR3H = 0x46; // 16-bit access (write high byte first, read low byte first)
	ICR3L = 0x50;

// Set Timer/Counter count/value registers (16 bit) TCNT1H and TCNT1L
	TCNT3H = 0; // 16-bit access (write high byte first, read low byte first)
	TCNT3L = 0;

// Initialise Channel A servo to mid-range position
	// Set Timer/Counter Output Compare Registers (16 bit) OCR3AH and OCR3AL
	// Pulse width ranges from 750uS to 2250uS
	// 'Neutral' (Mid range) pulse width 1.5mS = 1500uS pulse width
	OCR3A = 1500;
	
// Initialise Channel B servo to mid-range position
	// Set Timer/Counter Output Compare Registers (16 bit) OCR3BH and OCR3BL
	// Pulse width ranges from 750uS to 2250uS
	// 'Neutral' (Mid range) pulse width 1.5mS = 1500uS pulse width
	OCR3B = 1500;

// Initialise Channel C servo to mid-range position
	// Set Timer/Counter Output Compare Registers (16 bit) OCR3CH and OCR3CL
	// Pulse width ranges from 750uS to 2250uS
	// 'Neutral' (Mid range) pulse width 1.5mS = 1500uS pulse width
	OCR3C = 1500;

// TIMSK3 – Timer/Counter 3 Interrupt Mask Register
	// Bit 5 – ICIEn: Timer/Countern, Input Capture Interrupt Enable
	// Bit 3 – OCIEnC: Timer/Countern, Output Compare C Match Interrupt Enable
	// Bit 2 – OCIEnB: Timer/Countern, Output Compare B Match Interrupt Enable
	// Bit 1 – OCIEnA: Timer/Countern, Output Compare A Match Interrupt Enable
	// Bit 0 – TOIEn: Timer/Countern, Overflow Interrupt Enable
	TIMSK3 = 0b00000000;	// No interrupts needed, PWM pulses appears directly on OC3A, OC3B, OC3C (Port E Bits 3,4,5)
	
// TIFR3 – Timer/Counter3 Interrupt Flag Register
	TIFR3 = 0b00101111;		// Clear all interrupt flags
}