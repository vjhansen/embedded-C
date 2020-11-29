/*
1. Modify the code so that LED 3 flashes. Choose a different rate, 
    in-between the flash rates of LED 0 and LED 7.
*/

#include <avr/io.h>
#include <avr/interrupt.h>

void InitialiseGeneral();
void InitialiseTimer1();
void Toggle_LED0_1();
void Toggle_LED2_3();
void Toggle_LED4_5();
void Toggle_LED6_7();


// Declare global variables
unsigned char LED2_3_count;
unsigned char LED2_3_interval;

unsigned char LED4_5_count;
unsigned char LED4_5_interval;

unsigned char LED6_7_count;
unsigned char LED6_7_interval;


int main(void)
{
    InitialiseGeneral();
    
    LED2_3_count = 0;   // Initialise the count to 0
    LED2_3_interval = 4;    /* Set the interval for LED2 and 3 blinking
                                try with different values here */
    
    LED4_5_count = 0;           
    LED4_5_interval = 6;    // try with different values here
    
    LED6_7_count = 0;
    LED6_7_interval = 8;    // try with different values here
    
    InitialiseTimer1();
    
    while(1) {}
}

void InitialiseGeneral()
{
    DDRB  = 0xFF;           // Configure PortB direction for Output
    PORTB = 0xFF;           // Set all LEDs initially off (inverted on the board, so '1' = off)
    sei();
}

void InitialiseTimer1()     // Configure to generate an interrupt after a 1-Second interval
{
    TCCR1A = 0x00;          // Normal port operation (OC1A, OC1B, OC1C), Clear Timer on 'Compare Match' (CTC) waveform mode)
    TCCR1B = 0b00001101;    // CTC waveform mode, use prescaler 1024
    TCCR1C = 0x00;
    
    // For 1 MHz clock (with 1024 prescaler) to achieve a 1 second interval:
    // Need to count 1 million clock cycles (but already divided by 1024)
    
    // So actually need to count to (500k / 1024) = 0x1E8
    OCR1AH = 0x01; // Output Compare Registers (16 bit) OCR1BH and OCR1BL
    OCR1AL = 0xE8;

    TCNT1H = 0x00;  // Timer/Counter count/value registers (16 bit) TCNT1H and TCNT1L
    TCNT1L = 0x00;
    TIMSK1 = 0b00000010;    // bit 1 OCIE1A     Use 'Output Compare A Match' Interrupt, i.e. generate an interrupt
    // when the timer reaches the set value (in the OCR1A register)
}

ISR(TIMER1_COMPA_vect) // TIMER1_CompareA_Handler (Interrupt Handler for Timer 1)
{
    Toggle_LED0_1();        // Bit 0 is toggled every time the interrupt occurs
    
    LED2_3_count++;
    if(LED2_3_count == LED2_3_interval)     // Have we reached the required number of interrupts?
    {
        LED2_3_count = 0;                   // Reset the software counter for this event
        Toggle_LED2_3();
    }
    
    // SoftwareDivider logic, counts a number of interrupts, before taking action
    LED4_5_count++;
    if(LED4_5_count == LED4_5_interval)     // Have we reached the required number of interrupts?
    {
        LED4_5_count = 0;       
        Toggle_LED4_5();    
    }


    LED6_7_count++;
    if(LED6_7_count == LED6_7_interval)     // Have we reached the required number of interrupts?
    {
        LED6_7_count = 0;
        Toggle_LED6_7();
    }

    // Reset the count value back to zero (needed when using 'normal mode')
    // Alternatively, if CTC mode is used the timer is automatically cleared
    TCNT1H = 0x00;
    TCNT1L = 0x00;
}

void Toggle_LED0_1()    // toggle the value of port B bit 0 and 1 (if 0, set to 1; if 1 set to 0)
{   
    PORTB ^= (1 << PINB0); // toggle portB bit 0 (see this for explanation: http://www.rjhcoding.com/avrc-bit-manip.php)
    PORTB ^= (1 << PINB1);  
}

void Toggle_LED2_3()    // toggle the value of port B bit 1 and 3
{
    PORTB ^= (1 << PINB2);
    PORTB ^= (1 << PINB3);
}

void Toggle_LED4_5()    // toggle the value of port B bit 4 and 5
{
    PORTB ^= (1 << PINB4);
    PORTB ^= (1 << PINB5);
}

void Toggle_LED6_7()    // toggle the value of port B bit 6 and 7
{
    PORTB ^= (1 << PINB6);
    PORTB ^= (1 << PINB7);
}