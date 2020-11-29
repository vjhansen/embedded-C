/*
Part 2 is based on the TimerDemo2_SingleTimer_LED_MovingBar_2560 program.

    1. Modify the code so that the bar moves at a noticeably different speed.
    2. Modify the solution above so that the bar moves in the opposite direction.
*/


#include <avr/io.h>
#include <avr/interrupt.h>

void InitialiseGeneral();
void InitialiseTimer0();

unsigned int LED_value;

int main(void)
{
    InitialiseGeneral();
    LED_value = 256;        // Set initial LED value (leftmost bar is on)
    InitialiseTimer0();
    
    while(1) {}
}

void InitialiseGeneral()
{
    DDRB = 0xFF;            // Configure PortB direction for Output
    PORTB = 0xFF;           // Set all LEDs initially off (inverted on the board, so '1' = off)
    sei();
}

void InitialiseTimer0()     // Configure to generate an interrupt after a 1/4 Second interval
{
    TCCR0A = 0x00;  // Normal port operation (OC0A, OC0B), Normal waveform generation
    
    //TCCR0B = 0b00000101;  // Normal waveform generation, Use 1024 prescaler
    //TCCR0B = 0b00000010;  // clock/8
    //TCCR0B = 0b00000011;  // clk/64
    TCCR0B = 0b00000100;    // clk/256

    // For 1 MHz clock (with 1024 prescaler) Overflow occurs after counting to 256
    // (but already divided by 1024)
    // So overflow occurs after 1024 * 256 / 1000000 = 0.26 seconds
    // So approximately 4 bar movements will happen in 1 second
    
    TCNT0  = 0x00;  // Timer/Counter count/value register
    TIMSK0 = 0x01;      // Use 'Overflow' Interrupt, i.e. generate an interrupt
    // when the timer reaches its maximum count value
}

ISR(TIMER0_OVF_vect) // TIMER0_Overflow_Handler (Interrupt Handler for Timer 0)
{   // Shift LED BAR left
    LED_value /= 2;     // Shift left (SHL) by multiplying by 2 ___ SHR by dividing by 2
    if(0 == LED_value)  // Check if bar has now gone past 0
    {                   
        LED_value = 256;
    }
    PORTB = ~LED_value;
}