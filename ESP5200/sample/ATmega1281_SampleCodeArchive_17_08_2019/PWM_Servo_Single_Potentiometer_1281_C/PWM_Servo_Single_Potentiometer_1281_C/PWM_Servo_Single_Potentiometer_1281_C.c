//*****************************************
// Project 		SERVO control (Single) Demonstration using Pulse-Width-Modulation (C)
// Target 		ATmega1281 on STK300
// Program		PWM_Servo_Single_Potentiometer_1281_C.c
// Author		Richard Anthony
// Date			19th October 2013 (original 9th November 2011)

// Function		Controls position of a SERVO by changing the pulse width of the control signal
//				Timer 3 is used to produce a PWM signal which appears on Port E pin 3
//				Uses 'FAST PWM' mode of 16-bit Timer3 and the '1' prescaler.

//				Pulse width is controlled by a potentiometer connected to AtoD channel 2
//				i.e. a variable resistor (Potentiometer) is used to move the servo (full servo movement is approx 180 degrees)

//				PWM signals
//					PWM #1 Timer3 OC3A, Signal appears on PORT E bit 3
//					Potentiometer on PORT F bit 2 provides position signal which is converted into equivalent PWM signal

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
//					PORT B Configured for output (LEDs for diagnostic display of Potentiometer values)

//					Timer3 OC3A (Output Compare Match Output 3A) Port E bit 3
//					Port E bit 3 DDE3 must be configured as an output

//					Timer3 Input Capture Pin (ICP3) Port E bit 7 must be configured 
//					as an output to prevent random / spurious values entering the ICR3 register

//					Potentiometer on PORT F bit 2 (Analogue input)
//					PORT F configured for Analogue input on bit 2

//					Servos need external power supply since they can draw up to 550mA
//					which is more than the STK300 board can source.
//*****************************************

#include <avr/io.h>
#include <avr/interrupt.h>

void InitialiseGeneral(void);
void InitialiseTimer3_FastPWM_Single(void);
void Initialise_ADC(void);

unsigned char RESULTANT_DISPLAY_PATTERN;

int main(void)
{
	InitialiseGeneral();
	InitialiseTimer3_FastPWM_Single();
	Initialise_ADC();
	
	while(1)
	{
	}
}

void InitialiseGeneral()
{
	DDRB = 0xFF;	// Configure PortB direction for Output (LEDs for diagnostic display of ADC readings)
	PORTB = 0xFF;	// LEDs initially Off
	
	DDRE = 0xFF;	// Port E bit 3 must be set as OUTPUT to provide the PWM pulse on OC3A
					// Port E bit 7 Input Capture 3 Pin (ICP3) must be set as OUTPUT to prevent 
					// random / noise values entering ICR3 (ICR3 used as TOP value for PWM counter)
	PORTE = 0x00;	// Clear all bits initially
	
	// PORT F bit 2 is set automatically for Analogue input when the ADC module is configured
		
	sei();			// Enable interrupts at global level set Global Interrupt Enable (I) bit
}

void InitialiseTimer3_FastPWM_Single()
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
	
// TIMSK3 – Timer/Counter 3 Interrupt Mask Register
	// Bit 5 – ICIEn: Timer/Countern, Input Capture Interrupt Enable
	// Bit 3 – OCIEnC: Timer/Countern, Output Compare C Match Interrupt Enable
	// Bit 2 – OCIEnB: Timer/Countern, Output Compare B Match Interrupt Enable
	// Bit 1 – OCIEnA: Timer/Countern, Output Compare A Match Interrupt Enable
	// Bit 0 – TOIEn: Timer/Countern, Overflow Interrupt Enable
	TIMSK3 = 0b00000000;	// No interrupts needed, PWM pulses appears directly on OC3A, OC3B (Port E Bits 3,4)
	
// TIFR3 – Timer/Counter3 Interrupt Flag Register
	TIFR3 = 0b00101111;		// Clear all interrupt flags
}

void Initialise_ADC()
{
// ADMUX – ADC Multiplexer Selection Register
// bit7,6 Reference voltage selection (00 AREF,01 AVCC, 10 = Internal 1.1V, 11 = Internal 2.56V)
// bit 5 ADC Left adjust the 10-bit result
// 0 = ADCH (high) contains bit 1 = output bit 9, bit 0 = output bit 8. ADCL (low) contains output bits 7 through 0
// 1 = ADCH (high) contains bits 9 through 2. ADCL (low) contains bit 7 = output bit 1, bit 6 = output bit 0
// Bits 4:0 – MUX4:0: Analog Channel and Gain Selection Bits (see 1281 manual p290)
// 00000 = ADC0 (ADC channel 0, single-ended input)
// 00010 = ADC2 (ADC channel 2, single-ended input)
	ADMUX = 0b01100010;	// AVCC REF, Left-adjust output (Read most-significant 8 bits via ADCH), Convert channel 2

// ADCSRA – ADC Control and Status Register A
// bit 7 ADEN (ADC ENable) = 1 (Enabled)
// bit 6 ADSC (ADC Start Conversion) = 0 (OFF initially)
// bit 5 ADATE (ADC Auto Trigger Enable) = 1 (ON)
// bit 4 ADIF (ADC Interrupt Flag) = 0 (not cleared)
// bit 3 ADIE (ADC Interrupt Enable) = 1 (Enable the ADC Conversion Complete Interrupt)
// bit 2,1,0 ADC clock prescaler
// 000 = division factor 2
// 001 = division factor 2
// 010 = division factor 4
// 011 = division factor 8
// 100 = division factor 16
// 101 = division factor 32
// 110 = division factor 64
// 111 = division factor 128
	ADCSRA = 0b10101101;	// ADC enabled, Auto trigger, Interrupt enabled, Prescaler = 32

// ADCSRB – ADC Control and Status Register B
// Bit 3 – MUX5: Analog Channel and Gain Selection Bit (always 0 when using ADC0 - ADC7)
// Bit 2:0 – ADTS2:0: ADC Auto Trigger Source (active when ADATE bit in ADCSRA is set)
// 0 0 0 Free Running mode
// 0 0 1 Analog Comparator
// 0 1 0 External Interrupt Request 0
// 0 1 1 Timer/Counter0 Compare Match A
// 1 0 0 Timer/Counter0 Overflow
// 1 0 1 Timer/Counter1 Compare Match B
// 1 1 0 Timer/Counter1 Overflow
// 1 1 1 Timer/Counter1 Capture Event
	ADCSRB &= 0b11110000;	// clear bits 3,2,1,0 (Free running mode)

// DIDR0 – Digital Input Disable Register 0
// Bit 7:0 – ADC7D:ADC0D: ADC7:0 Digital Input Disable
	DIDR0 = 0b00000100;	// Disable digital input on bit 2

// DIDR2 – Digital Input Disable Register 2
// Bit 7:0 – ADC15D:ADC8D: ADC15:8 Digital Input Disable
	DIDR2 = 0b11111111;	// Disable digital input on all bits (64-pin version of ATmega1281 does not even have these inputs)

// Start the ADC Conversion (start first sample, runs in 'free run' mode after)
	//bit 6 ADCSRA (ADC Start Conversion) = 1 (START)
	// Read ADSCSR and OR with this value to set the flag without changing others
	ADCSRA |= 0b01000000;	// start ADC conversion
}

ISR(ADC_vect)	// ADC Interrupt Handler
{	
	unsigned char ADCH_temp = ADCH;
	
	// ADC2_HANDLER
	// Convert ADC value into Servo Pulse width for PWM channel A
	// Adjust the Most-significant 8-bits of the input value to a suitable position pulse-width value
	// The ADC input (values 0 - 255) need to represent an pulse-width value in the 1500us-wide range 750us to 2250us
	// A simple approximation is used:
	// Multiply by 6, to get a value in range 0 - 1536, and then add the 750 offset.
	OCR3A = (ADCH_temp * 6) + 750;

	PORTB = ~ADCH_temp;		// Display the ADC value onto the LEDs for diagnostic
}
