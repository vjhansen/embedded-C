// Project      Timer Demo #4 Use a Timer to measure the time interval between events
// Program      TimerDem4_MeasuringTimeBetweenEvents_1281_C.c
// Author       Richard Anthony - 15th September 2013

// Function     Demonstrates how to use a programmable timer to measure a time interval
//                  In this demonstration the timer is initially 'configured' but is not 'started'.
//                  The first 'event' causes the timer to be started
//                  The second 'event' causes the timer to be stopped and the elapsed time
//                  between the two events is displayed on the on-board LEDs.

//              Demonstrates Hardware Interrupts
//                 The first 'event' that starts the timer is Hardware Interrupt INT0
//                  (alternate function of port D bit 0). By connecting the switches of
//                  the external Switches and Lights box to port D we can use the bit 0
//                  switch to generate an interrupt on pin INT0.

//                 The second 'event' that stops the timer is Hardware Interrupt INT1
//                  (alternate function of port D bit 1). By connecting the switches of
//                  the external Switches and Lights box to port D we can use the bit 1
//                  switch to generate an interrupt on pin INT1.

//  Timer 'range'   Uses 16-bit Counter/Timer 1 (CT1)
//                  This timer can measure a time period of up to 65535 clock pulses
//                  (i.e.2 to the power of 16, minus 1)
//                  (This is the biggest number we can load into the timer's register)

//                  If the system clock is used directly
//                      If the clock runs at 1Mhz, the maximum time period is
//                      65535 / 1000000 = 65.5 Milliseconds
//  

//  Prescaler       The prescaler options for timer 1 are:
//                  1 (no prescaling), 8, 64, 256 or 1024

//                  If the 1024 prescaler is used
//                      If the clock runs at 1Mhz, the maximum time period is
//                      (65535 * 1024) / 1000000 = 67 Seconds

//                  If the time period between events exceeds the maximum timeable period,
//                  the Timer Overflow Interrupt will be triggered and will flash the LEDs
//                  to signal that the timer has overflowed

// I/O hardware Uses on-board LEDs on Port K to display the time interval, in seconds
//                  Uses External Switches on Port D to provide the start / stop triggers

//                  While counting, displays 'live' the number of elapsed seconds between the start and stop events
//                  After stopped, displays the 'frozen' number of elapsed seconds between the start and stop events


/*

TASK:
Part 4 is based on the TimerDemo4_MeasuringTimeBetweenEvents_2560 program.

    Modify the code to reverse the control logic, so that Hardware Interrupt 1 (INT1 = PD1) starts the timer, and
    HW Interrupt 0 (INT0 = PD0) stops the timer and displays the elapsed time interval.
    
    Just take a wire from PD0 or PD1 and connect it to GND in order to enable the HW interrupt

*/


#include <avr/io.h>
#include <avr/interrupt.h>

void InitialiseGeneral();
void Initialise_HW_Interrupts();
void InitialiseTimer1();
void DisplayLED();

// Declare global variables
unsigned char ElapsedSeconds_Count; // An 'unsigned char' is an 8-bit numeric value, i.e. a single byte

int main(void)
{
    ElapsedSeconds_Count = 0x00;        // Initialise the Elapsed Time counter
    Initialise_HW_Interrupts();
    InitialiseTimer1();
    InitialiseGeneral();
    
    while(1) {}
}

void InitialiseGeneral()
{
    DDRK = 0xFF;            // Configure PortK direction for Output
    PORTK = 0x00;           // Set all LEDs initially off
    DDRD = 0x00;            // Configure PortD direction for Input (PD1 and PD0)
    sei();
}

void Initialise_HW_Interrupts()
{
    EICRA = 0b00001010;     // INT 3,2 not used, Interrupt Sense (INT1, INT0) falling-edge triggered
    EICRB = 0x00;           // INT7 ... 4 not used
    
    EIMSK = 0b00000011;     // Enable INT1, INT0
    EIFR = 0b00000011;      // Clear INT1 and INT0 interrupt flags (in case a spurious interrupt has occurred during chip startup)
}


// Interrupt 0 (INT0 = PD0) stops the timer and displays the elapsed time interval.
ISR(INT0_vect)              // Hardware_INT0_Handler (Interrupt Handler for INT0 = PD0)
{
    TCCR1B = 0b00001000;    // Stop the timer (CTC, no clock)
    DisplayLED();           // Display the amount of time elapsed
}


// Interrupt 1 (INT1 = PD1) starts the timer
ISR(INT1_vect)              // Hardware_INT1_Handler (Interrupt Handler for INT1 = PD1)
{
    ElapsedSeconds_Count = 0;   // Clear the elapsed time counter
    DisplayLED();               // Clear the display
    TCCR1B = 0b00001101;        // Start the timer (CTC and Prescaler 1024)
    
}

void InitialiseTimer1()     // Configure to generate an interrupt after a 1-Second interval
{
    TCCR1A = 0x00;  // Normal port operation (OC1A, OC1B, OC1C), Clear Timer on 'Compare Match' (CTC) waveform mode)
    TCCR1B = 0b00001000;    // CTC waveform mode, initially stopped (no clock)
    TCCR1C = 0x00;
    
    // For 1 MHz clock (with 1024 prescaler) to achieve a 1 second interval:
    // Need to count 1 million clock cycles (but already divided by 1024)
    // So actually need to count to (1000000 / 1024 =) 976 decimal, = 3D0 Hex
    OCR1AH = 0x03; // Output Compare Registers (16 bit) OCR1BH and OCR1BL
    OCR1AL = 0xD0;

    TCNT1H = 0x00;  // Timer/Counter count/value registers (16 bit) TCNT1H and TCNT1L
    TCNT1L = 0x00;
    TIMSK1 = 0b00000010;    // bit 1 OCIE1A     Use 'Output Compare A Match' Interrupt, i.e. generate an interrupt
    // when the timer reaches the set value (in the OCR1A register)
}

ISR(TIMER1_COMPA_vect) // TIMER1_CompareA_Handler (Interrupt Handler for Timer 1)
{
    ElapsedSeconds_Count++; // Increment the number of elapsed seconds while the timer has been running
    DisplayLED();           // Display the amount of time elapsed
}

void DisplayLED()
{
    unsigned char Output_LED_value;
    Output_LED_value = ElapsedSeconds_Count;
    PORTK = Output_LED_value;
}
