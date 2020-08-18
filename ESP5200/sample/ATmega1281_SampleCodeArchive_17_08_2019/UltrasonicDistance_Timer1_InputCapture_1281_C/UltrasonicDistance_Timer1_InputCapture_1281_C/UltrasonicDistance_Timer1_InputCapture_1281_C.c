//*******************************************************************************
// Project 		Ultrasonic Distance Measurement - uses Timer1 Input Capture Pin (Embedded C)
// Target 		ATMEL ATmega1281 micro-controller on STK300 board
// Program		UltrasonicDistance_Timer1_InputCapture_1281_C.c
// Author		Richard Anthony
// Date			7th November 2017

// Function			Demonstrates how to use the Input Capture mode on programmable Timer1
//					Demonstrates use of Hardware Interrupt 3
//					Demonstrates dynamic IO port direction alternation via bit-wise IO
// 					Demonstrates the operation of the Parallax 'PING)))' Ultrasonic Distance Sensor (US)

//	Operation
//					Configuration:
//						*** Note: The US signal pin must be connected to Port D bits 3 AND 4 ***
//
//						Configure Timer1 for Input Capture mode (PORTD bit4), mode - stop timer on low-going edge
//						Configure Hardware Interrupt 3 (PORTD bit3) - generate interrupt on rising edge
//						Configure DDRB for output (LEDs connected to PORTB)
//
//						(configuration has been done, now send the trigger pulse to the US)
//
//						*** Start measurement episode ***
//						Wait a short interval (the Ultrasonic sensor requires 200uS interval between sensing episodes)
//						Set DDRD bit 4 for output direction
//						Generate pulse (5uS) on PORTD bit 4 to trigger generation of the US Ping pulse
//						Set DDRD bit 4 for input direction
//						Enable H/W interrupt 3 - to detect the leading edge of the US echo pulse
//
//						(trigger pulse has been sent, now waiting for the start of the echo pulse)
//
//					Hardware Interrupt 3 Interrupt Handler:
//						(The interrupt occurs when the leading edge of the US echo pulse is detected)
//						Start Timer 1 counting (1MHz clock used, therefore counts in 1uS increments)
//
//						(start of the echo pulse has been detected, now waiting for the end of the echo pulse)
//
//					TIMER1 Input Capture Interrupt Handler:
//						(Echo pulse width represents object distance. Pulse width 115uS = 2cm to pulse width 18500uS = 300cm)
//						(Echo pulse high whilst active, low going edge signals the end of the echo pulse, which stops the timer)
//
//						(end of the echo pulse has been detected)
//
//						When the falling-edge is detected (by the Input Capture mechanism) the timer will stop counting
//						Convert pulse width to distance indication
//						Display distance indication on LEDs (via PORTB)
//						Start next measurement episode 
//
//					TIMER1 CompareA Interrupt Handler:
//						The timer has been configured to timeout if the falling edge of the echo pulse is not detected.  
//						(the timer will count beyond the maximum echo pulse time limit, and cause this interrupt to trigger)
//						Display diagnostic pattern on the LEDs (via PORTB) bit 7 used	

//		Ports
//			Port B - LEDs Output
//			Port D - Hardware Interrupt 3 (bit 3) Input
//			Port D - Timer 1 Input Capture (bit 4), also used to generate trigger pulse (thus alternate Input and Output direction)

//			*** Note that the Ultrasonic Distance Sensor signal pin must be connected to Port D bits 3 AND 4 ***
//			There are THREE different actions on port D bits 3 and 4:
//			1. using bit 4 - generate the short trigger pulse (the bit is set for output direction for this action)
//			2. using bit 3 - detect the echo pulse leading (rising) edge (the bit is always set for input direction)
//			3. using bit 4 - detect the echo pulse ending (falling) edge (the bit is set for input direction for this action)
//*****************************************
#define F_CPU 1000000
#include <avr/io.h>
#include <avr/interrupt.h>
#include <util/delay.h>

// Declare functions (these will be in a separate header file in larger programs)
void InitialiseGeneral();
void Initialise_INT3();
void Enable_INT3();
void Disable_All_Hardware_Interrupts();
void InitialiseTimer1_InputCapture_CompareA();
void Enable_Timer1_Interrupts();	
void Disable_Timer1_Interrupts();	
void Generate_US_Sensor_Trigger_Pulse();

int main( void )
{
	InitialiseGeneral();
	Initialise_INT3();
	InitialiseTimer1_InputCapture_CompareA();

	Generate_US_Sensor_Trigger_Pulse(); // Start first measurement episode
	
    while(1)        	// Loop
    {
	}
}

void InitialiseGeneral()
{
	DDRB = 0xFF;		// Configure PortB direction for Output
	PORTB = 0xFF;		// Set all LEDs initially off (inverted on the board, so '1' = off)	

	DDRD = 0x00;		// Configure PortD direction for Input (bits 3 and 4 connected to US sensor - pullup resistors not needed)
						// H/W interrupt 3 (bit 3) - will always be an input 
						// Echo pulse detect (Timer1 Input Capture bit 4), will be dynamically changed between input and output in the code
	
	sei();				// Enable interrupts at global level, set Global Interrupt Enable (I) bit
}

void Initialise_INT3()
{
	Disable_All_Hardware_Interrupts();	// Hardware interrupts disabled initially

	// EICRA – External Interrupt Control Register A
	// Bits 7:0 – ISC31, ISC30 – ISC00, ISC00: External Interrupt 3 - 0 Sense Control Bits
	// 11  = The rising edge of INTn generates asynchronously an interrupt request
	EICRA = 0b11000000;		// Interrupt Sense (INT3) rising-edge triggered
	
	// EICRB – External Interrupt Control Register B
	// Bits 7:0 – ISC71, ISC70 - ISC41, ISC40: External Interrupt 7 - 4 Sense Control Bits
	EICRB = 0b00000000;
}

void Enable_INT3()
{
	EIFR = 0b11111111;		// Clear all HW interrupt flags
	EIMSK = 0b00001000;		// Enable H/W Int 3
}

void Disable_All_Hardware_Interrupts()
{
	EIMSK = 0b00000000;		// All H/W interrupts disabled
	EIFR = 0b11111111;		// Clear all HW interrupt flags
}

ISR(INT3_vect) // Interrupt Handler for H/W INT 3
{	// Here on rising edge at start of Ultrasonic Sensor pulse output
	TCNT1 = 0;			// Restart the timer count
	ICR1 = 0;			// Clear Input Capture Register

	Disable_All_Hardware_Interrupts();		// The pulse leading (rising) edge has been detected
	Enable_Timer1_Interrupts();				// Now wait for the training (falling) edge
}

void InitialiseTimer1_InputCapture_CompareA()	// Measure the time length of the Ultrasonic Distance Sensor echo pulse
{
	Disable_Timer1_Interrupts();	// All Timer 1 interrupts disabled initially

	TCCR1A = 0b00000000; // Normal port operation (OC1A, OC1B, OC1C), normal waveform mode
	TCCR1B = 0b00000001; // Normal waveform mode, bit6 Input Capture Edge Select 0 = falling edge, prescaler 1 (count 1uS increments, 1MHz clock)
	TCCR1C = 0b00000000;
	
	OCR1A = 18500;	// Output Compare Register A
					// Maximum echo response pulse width is 18500uS
					// CompareA Interrupt Handler used to Stop timer if no echo pulse received within this timeframe

	TCNT1 = 0;		// Timer/Counter count/value register
	ICR1 = 0;		// Input Capture Register
}
 
void Enable_Timer1_Interrupts()
{
	TIFR1 =  0b00101111;	// Clear all timer 1 interrupt flags
	TIMSK1 = 0b00100010;	// Enable timer 1 Interrupts
							// bit 5 ICIE1		Input Capture Interrupt - detect end (falling edge) of US sensor echo pulse
							// bit 1 OCIE1A		Output Compare A Match Interrupt - detect missing US sensor echo pulse	
}

void Disable_Timer1_Interrupts()
{
	TIMSK1 = 0b00000000;	// All Timer 1 interrupts disabled
	TIFR1 =  0b00101111;	// Clear all timer 1 interrupt flags
}
 
ISR(TIMER1_CAPT_vect) // TIMER1 Input Capture Interrupt Handler
{	// Here when the falling edge of the echo pulse is detected by the ICP1 hardware logic
	long lPulseWidth = ICR1;	// Read the pulse width value from the timer's Input Capture register
	Disable_Timer1_Interrupts();
	Disable_All_Hardware_Interrupts();	

	// The Ultrasonic Distance sensor generates echo pulse widths between 115uS and 18500uS
	// These equate to distances 2cm to 300cm (3m)
	
	// The pulse width is related to the object distance by the formula		D = P * 300 / 18500
	// where P is the actual measured pulse width, and D is the distance in cm
	
	// Some examples:	P = 18500, D = 18500 * 300 / 18500 = 300cm
	//					P =  9250, D = 9250 * 300 / 18500 = 150cm
	//					P =  1850, D = 1850 * 300 / 18500 = 30cm
	//					P =  1850, D = 1850 * 300 / 18500 = 30cm
	//					P =  617, D = 617 * 300 / 18500 = 10cm
	//					P =  123, D = 123 * 300 / 18500 = 2cm

	// Note - this formula, and these values, give a reasonable indication of distance, approximately accurate to 1cm
	// *** It is recommended that users perform their own empirical calibrations if high accuracy is needed ***

	long lDistance = 0;
	if(115 <= lPulseWidth && 18500 >= lPulseWidth)
	{	// Pulse width is in expected range (otherwise ignore it)
		
//		// Note - Display Resolution (using 8 LEDs)
//		// 1cm resolution - can display distances up to 255cm (the count value displayed on the LEDs represents cm distance)
//		lDistance = 1 + ((lPulseWidth * 300) / 18500);

	
		// Alternative configuration - Display Resolution (using 8 LEDs)
		// 1mm resolution - can display distances up to 25.5cm (the count value displayed on the LEDs represents mm distance)
		lDistance = 10 + ((lPulseWidth * 3000) / 18500);

		if(255 < lDistance)
		{
			lDistance = 255;	// Display all higher distances as the maximum value that can be represented
		}
	}

	PORTB = ~lDistance;
	
	Generate_US_Sensor_Trigger_Pulse();	// *** Start next measurement episode ***
}

ISR(TIMER1_COMPA_vect) // TIMER1 CompareA Interrupt Handler
{	// Here if end of echo pulse is detected (should not occur)
	// The timer has counted to a value equivalent to greater than the maximum detection distance
	Disable_Timer1_Interrupts();
	Disable_All_Hardware_Interrupts();
		
	PORTB = (unsigned char) ~0b10000000;	// Signal that there was no echo pulse (no object in distance range)
		
	Generate_US_Sensor_Trigger_Pulse();	// *** Start next measurement episode ***
}

void Generate_US_Sensor_Trigger_Pulse()
{	// Generate a short pulse to the US distance sensor to start a measurement episode
	Disable_Timer1_Interrupts();		// Safety measure (whether pulse detected or timeout occurred, Interrupt should already be disabled)
	Disable_All_Hardware_Interrupts();	// Safety measure (whether pulse detected or timeout occurred, Interrupt should already be disabled)
	
	_delay_us(200);		// Delay 200uS must occur between measurement episodes, see US sensor manual
						// Use of the delay is acceptable here because it is very short and far less complex than using 
						// a second programmable timer to achieve the same effect, in this case)
	
	// The sequence to change the I/O pin configuration and generate the pulse
	DDRD |= 0b00010000;	// Configure PortD bit 4 direction for Output
	PORTD = 0b00010000;	// Start the trigger pulse
	// Short pulse width - about 5uS is ideal (this is achieved when using C without any added delay - confirmed empirically with oscilloscope)
	PORTD = 0b00000000;	// End the trigger pulse
	DDRD &= 0b11101111;	// Configure PortD bit 4 direction for Input (wait for echo pulse, detected by Timer1 Input Capture)

	Enable_INT3();		// Enable the interrupt to detect the rising edge of the US sensor output pulse
}