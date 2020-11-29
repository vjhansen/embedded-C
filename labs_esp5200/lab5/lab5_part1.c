/*

 Part 1 is based on the TimerDemo1_SingleTimerSingleLED_PB7_Flash_2560_C program
 
 - Modify the code so that the LED flashes twice as fast. 
        Flashes the LED at 2 Hz (2 flashes per second)
 - Modify the solution above so that two LEDs flash together. 

*/ 


#include <avr/io.h>
#include <avr/interrupt.h>

#define F_CPU 1000000UL

// Declare functions (these will be in a separate header file in larger programs)
void InitialiseGeneral();
void InitialiseTimer1();

// Declare global variables
unsigned char LED_value;    // An 'unsigned char' is an 8-bit numeric value, i.e. a single byte
unsigned char i;

int main(void)
{
    InitialiseGeneral();
    LED_value = 0x00;       // Initialise the variable
    i = 0x01;
    InitialiseTimer1();
    
    while(1) {}
}

void InitialiseGeneral()
{
    DDRB  = 0xFF;           // Configure PortB direction for Output
    PORTB = 0xFF;           // Set all LEDs initially off (inverted on the board, so '1' = off) 
    DDRD  = 0x00;           // Configure PortD direction for Input

    sei();                  // Enable interrupts at global level, set Global Interrupt Enable (I) bit
}

void InitialiseTimer1()     // Configure to generate an interrupt after a 1-Second interval
{
    TCCR1A = 0x00;          // Normal port operation (OC1A, OC1B, OC1C), Clear Timer on 'Compare Match' (CTC) waveform mode)
    TCCR1B = 0b00001101;    // CTC waveform mode, use prescaler 1024
    TCCR1C = 0x00;
    
    
    // 1 clock cycle = 1 us __ 1 second = 1000 ms =  1 000 000 us
    // 8-bit counter -> 255 __ 16-bit counter -> 65535
    
    // For 1 MHz clock (with 1024 prescaler) to achieve a 1 second interval:
    // Need to count 1 million clock cycles (but already divided by 1024)
    // So actually need to count to (1000000 / 1024 =) 976 decimal, = 3D0 Hex
    
    // 0.5 second interval --> 500 000 us
    // Need to count 500k clock cycles (but already divided by 1024)
    // So actually need to count to (500 000 / 1024 =) 488 decimal, = 1E8 Hex
    
    // Output Compare Registers (16 bit) OCR1BH and OCR1BL
    /* 1 Hz*/
    //OCR1AH = 0x03;
    //OCR1AL = 0xD0;

    /* 2 Hz*/
    OCR1AH = 0x01;
    OCR1AL = 0xE8;
    
    TCNT1H = 0x00;  // Timer/Counter count/value registers (16 bit) TCNT1H and TCNT1L
    TCNT1L = 0x00;
    TIMSK1 = 0x02;  // bit 1 OCIE1A     Use 'Output Compare A Match' Interrupt, i.e. generate an interrupt
                            // when the timer reaches the set value (in the OCR1A register) 
}

ISR(TIMER1_COMPA_vect) // TIMER1_CompareA_Handler (Interrupt Handler for Timer 1)
{
    // @Deivy, you can add your logic here, just remove mine
    i++;
    if (i > 0x0f)
        i = 1;
    
    if (0 == LED_value) {
        LED_value  |= ( (1 << (i-1))|(1 << i) );
    }
    else
        LED_value = 0;
    PORTB = LED_value;  // Flip the value
}