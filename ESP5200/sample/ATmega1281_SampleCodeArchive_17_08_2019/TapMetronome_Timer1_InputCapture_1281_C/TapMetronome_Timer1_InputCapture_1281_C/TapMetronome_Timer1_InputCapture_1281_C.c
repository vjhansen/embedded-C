//*******************************************************************************
// Project 		Tap Metronome - uses Timer1 Input Capture Pin (Embedded C)
// Target 		ATMEL ATmega1281 micro-controller on STK300 board
// Program		TapMetronome_Timer1_InputCapture_1281_C.c
// Author		Richard Anthony
// Date			5th November 2017

// Function		Demonstrates:
//					Use of Input Capture mode on programmable timer1
// 					Simple learning - analyses successive intervals between button presses, calculates frequency and generates a metronome effect on a LED
//					A simple state-machine

// Operation
//				Timer1 used in Input Capture mode (PORTD bit4), mode - stop timer on low-going edge
//				Timer5 used to generate the computed metronome frequency

// Ports
//				PORT B - LEDs
//				PORT D - on-board switches	- bit 4, Tap input (Timer1 Input Capture - Port D bit 4)
//											- bit 7, Stop metronome and listen for Tap sequence
//*****************************************
#define F_CPU 1000000
#include <avr/io.h>
#include <avr/interrupt.h>
#include <util/delay.h>
#include <stdbool.h>

// Push button function definitions
#define Switch_Tap_Input						0b00010000 /* Enter Tap sequence via Timer1 Input Capture (PORTD bit4). 4 taps (3 intervals) needed to detect the average interval */
#define Switch_Stop_Metronome_Listen_For_Tap	0b10000000 /* Stop metronome - puts system into 'listen-for-tap' mode */

// LED indications
#define LED_FirstTapDetected				0b00000001	// (1)
#define LED_SecondTapRecorded				0b00000010	// (2)
#define LED_ThirdTapRecorded				0b00000100	// (3)
#define LED_FourthTapRecorded_PlayMetronome	0b00001000	// (4)
#define LED_Pulse							0b00010000	// Pulses when Taps are input and also to signal the metronome output
#define LED_Stopped							0b10000000	// (S)

#define Max_Interval						20000		// The maximum time interval (in 1.024 mS units)

// Declare functions (these will be in a separate header file in larger programs)
void InitialiseGeneral();
void InitialiseTimer1_InputCapture_CompareA();
void InitialiseTimer5_IntervalGenerator(unsigned long ulInterval);
void StopTimer5();
void SetState_Status_Stopped_ListenForFirstTap();
void SetState_Status_FirstTapDetected();
void SetState_Status_SecondTapRecorded(long lPulseWidth);
void SetState_Status_ThirdTapRecorded(long lPulseWidth);
void SetState_Status_FourthTapRecorded_PlayMetronome(long lPulseWidth);
double ArithmeticMean_Running(double dNewValue, bool bReset);
void Pulse_LED4();

// The state machine comprises a number of defined states and the legal transitions between these states
// States are given descriptive names:
// Status_Stopped_ListenForFirstTap (S)
// Status_FirstTapDetected (1)
// Status_SecondTapRecorded (2)
// Status_ThirdTapRecorded (3)
// Status_FourthTapRecorded_PlayMetronome (4)

// The states are defined in the form of an enumeration
volatile enum Status {	Status_Stopped_ListenForFirstTap, Status_FirstTapDetected, Status_SecondTapRecorded, 
						Status_ThirdTapRecorded, Status_FourthTapRecorded_PlayMetronome} 
						eStatus;
// The legal transitions are:
// S (tap detected)> 1 
// S (stop button pressed)> S
// 1 (tap detected)> 2
// 1 (stop button pressed)> S
// 1 (timeout)> S
// 2 (tap detected)> 3
// 2 (stop button pressed)> S
// 2 (timeout)> S
// 3 (tap detected)> 4
// 3 (stop button pressed)> S
// 3 (timeout)> S
// 4 (stop button pressed)> S

// Notes relating to the states and transitions:
// - once 4 taps have been detected, tap inputs are ignored until the system has reverted back to S state
// - if a timeout occurs whilst waiting for a tap (states {1,2,3} no tap detected in timeout period) the system transitions to S state

int main()
{
	unsigned char ucSwitchesValue = 0;
	InitialiseGeneral();
	InitialiseTimer1_InputCapture_CompareA();
	SetState_Status_Stopped_ListenForFirstTap();
	
	while(1)
	{
		_delay_ms(100); // Simple switch debounce
		
		ucSwitchesValue = ~PIND;	// Read value on switches

		// Update the state based on the push switches value 
		switch(ucSwitchesValue)
		{
			case Switch_Tap_Input:
				// Switch_Tap_Input is detected via its direct connection to Timer 1 Input Capture
				// No action needed here
				break;

			case Switch_Stop_Metronome_Listen_For_Tap:
				SetState_Status_Stopped_ListenForFirstTap();
				break;
		}
	}
}

void InitialiseGeneral()
{
	DDRB = 0xFF;			// Configure PortB direction for Output
	PORTB = 0xFF;			// Set all LEDs initially off (inverted on the board, so '1' = off)	

	DDRD = 0x00;			// Configure PortD direction for Input (on-board switches have their own external pullup resistors)
	
	sei();					// Enable interrupts at global level, set Global Interrupt Enable (I) bit
}

void InitialiseTimer1_InputCapture_CompareA()	// Measure the time length of the Ultrasonic Distance Sensor echo pulse
{
	TCCR1A = 0b00000000; // Normal port operation (OC1A, OC1B, OC1C), normal waveform mode)
	TCCR1B = 0b00000101; // Normal waveform mode, bit6 Input Capture Edge Select 0 = falling edge, prescaler 1024 (1.024mS increments, 1MHz clock)
	TCCR1C = 0b00000000;
	
	OCR1A = Max_Interval;	// Output Compare Register A	Implement timeout

	TCNT1 = 0;				// Timer/Counter count/value register
	ICR1 = 0;				// Input Capture Register
	TIFR1 =  0b00101111;	// Clear all interrupt flags
	TIMSK1 = 0b00100010;	// bit 5 ICIE1		Use Input Capture Interrupt - to measure the interval between Taps 1-2, 2-3, 3-4
							// bit 1 OCIE1A		Use Output Compare A Match Interrupt - Timeout if interval greater than the defined maximum interval
}
 
ISR(TIMER1_CAPT_vect) // TIMER1 Input Capture Interrupt Handler
{	// Here when the Tap push-button falling edge is detected by the ICP1 hardware logic
	long lPulseWidth = ICR1;	// Read the expired time value from the timer's Input Capture register

	switch(eStatus) {
		case Status_Stopped_ListenForFirstTap:
			Pulse_LED4();
			SetState_Status_FirstTapDetected();
			break;
		case Status_FirstTapDetected:
			Pulse_LED4();
			SetState_Status_SecondTapRecorded(lPulseWidth);
			break;
		case Status_SecondTapRecorded:
			Pulse_LED4();
			SetState_Status_ThirdTapRecorded(lPulseWidth);
			break;
		case Status_ThirdTapRecorded:
			Pulse_LED4();
			SetState_Status_FourthTapRecorded_PlayMetronome(lPulseWidth);
			break;
		case Status_FourthTapRecorded_PlayMetronome:
			break;
	}
	_delay_ms(100); // Simple switch debounce
	TIFR1 =  0b00101111;	// Clear all interrupt flags (in case another switch press was detected whilst in debounce delay)
}

ISR(TIMER1_COMPA_vect) // TIMER1 CompareA Interrupt Handler (Timeout)
{	// Here if the tap input interval is greater than the defined maximum interval
	switch(eStatus) {
		case Status_Stopped_ListenForFirstTap:
			break;
		case Status_FirstTapDetected:
		case Status_SecondTapRecorded:
		case Status_ThirdTapRecorded:
			SetState_Status_Stopped_ListenForFirstTap();	// Timeout
			break;
		case Status_FourthTapRecorded_PlayMetronome:
			break;
	}
}

void InitialiseTimer5_IntervalGenerator(unsigned long ulInterval)
{
	TCCR5A = 0b00000000; // Normal port operation (OC1A, OC1B, OC1C), CTC waveform mode)
	TCCR5B = 0b00001101; // CTC waveform mode, prescaler 1024 (1.024mS increments, 1MHz clock)
	TCCR5C = 0b00000000;
	
	if(Max_Interval < ulInterval)
	{	// Enforce the maximum time interval
		ulInterval = Max_Interval;
	}
	
	OCR5A = ulInterval;	// Output Compare Register A - controls the time interval

	TCNT5 = 0;				// Timer/Counter count/value register
	TIFR5 =  0b00101111;	// Clear all interrupt flags
	TIMSK5 = 0b00000010;	// bit 1 OCIE5A		Use Output Compare A Match Interrupt - Timeout when interval complete
}

void StopTimer5()
{
	TIFR5 =  0b00101111;	// Clear all interrupt flags
	TIMSK5 = 0b00000000;	// Disable all interrupts
	TCCR5B = 0b00001000;	// CTC waveform mode, prescaler 0 (stopped)
}

ISR(TIMER5_COMPA_vect) // TIMER5 CompareA Interrupt Handler (Timeout)
{	// Pulse interval completed
	Pulse_LED4();
}

void SetState_Status_Stopped_ListenForFirstTap()
{
	eStatus = Status_Stopped_ListenForFirstTap;
	StopTimer5();
	TCNT1 = 0;			// Restart the timer count
	ICR1 = 0;			// Clear Input Capture Register
	PORTB = (unsigned char) ~LED_Stopped;	// Display state indicator diagnostic pattern
}

void SetState_Status_FirstTapDetected()
{	// The interval BEFORE the first tap is ignored - the tap is only used to start the sequence
	eStatus = Status_FirstTapDetected;
	TCNT1 = 0;			// Restart the timer count
	ICR1 = 0;			// Clear Input Capture Register
	PORTB = (unsigned char) ~LED_FirstTapDetected;	// Display state indicator diagnostic pattern
}

void SetState_Status_SecondTapRecorded(long lPulseWidth)
{
	eStatus = Status_SecondTapRecorded;
	TCNT1 = 0;			// Restart the timer count
	ICR1 = 0;			// Clear Input Capture Register
	ArithmeticMean_Running((double) lPulseWidth, true /*bReset*/); // Initialise running Arithmetic Mean, with first interval value
	PORTB = (unsigned char) ~LED_SecondTapRecorded;	// Display state indicator diagnostic pattern
}

void SetState_Status_ThirdTapRecorded(long lPulseWidth)
{
	eStatus = Status_ThirdTapRecorded;
	TCNT1 = 0;			// Restart the timer count
	ICR1 = 0;			// Clear Input Capture Register
	ArithmeticMean_Running((double) lPulseWidth, false /*bReset*/); // Update running Arithmetic Mean, with second interval value
	PORTB = (unsigned char) ~LED_ThirdTapRecorded;	// Display state indicator diagnostic pattern
}

void SetState_Status_FourthTapRecorded_PlayMetronome(long lPulseWidth)
{
	unsigned long ulMeanInterval = 0;
	eStatus = Status_FourthTapRecorded_PlayMetronome;
	ulMeanInterval = (unsigned long) ArithmeticMean_Running((double) lPulseWidth, false /*bReset*/); // Update running Arithmetic Mean, with third interval value
	InitialiseTimer5_IntervalGenerator(ulMeanInterval);	// Configure Timer 5 to generate pulses at a rate corresponding to the detected mean interval
	PORTB = (unsigned char) ~LED_FourthTapRecorded_PlayMetronome;	// Display state indicator diagnostic pattern
	_delay_ms(50);
	PORTB = 0xFF;	// Clear the state indicator after a short time, so that only the pulse indicator is shown when metronome is running
}

double ArithmeticMean_Running(double dNewValue, bool bReset)
{	// Compute a running Arithmetic Mean
	static double dMean;
	static long lCount;
	double dWorkingTotal;
	if(true == bReset)
	{	// The first value is initially the mean
		dMean = dNewValue;
		lCount = 1;
	}
	else
	{
		// Compute the new mean
		dWorkingTotal = dNewValue + (dMean * lCount);
		lCount ++;
		dMean = dWorkingTotal / lCount;
	}
	return dMean;
}

void Pulse_LED4()
{
	PORTB &= ~LED_Pulse; // on-board LEDs are inverted; to turn LED4 ON need to bitwise AND with inverted mask pattern
	_delay_ms(60);
	PORTB |= LED_Pulse; // on-board LEDs are inverted; to turn LED4 OFF need to bitwise OR with non-inverted mask pattern
}