// Project      Timer Demo5 Uses all three timers to flash three LEDs at different speeds (Embedded C)
// Program      TimerDemo5_3Timers_FlashSeparateLEDs_1281_C.c
// Author       Richard Anthony - 15th September 2013

// Function         Demonstrates how to use the programmable timers
//                  Three timers are used (0, 1, 3), each one flashes a single LED at a specific rate


// I/O hardware     Uses on-board LEDs on Port K (output)


/*
Part 5 is based on the TimerDemo5_3Timers_FlashSeparateLEDs_2560 program.
    1. Experiment with changing the flash rate of each timer.
    2. Modify the code so that each timer controls two LEDs so that they alternate (i.e. for each timer, one of its two LEDs always has the opposite value of the other)
*/



#include <avr/io.h>
#include <avr/interrupt.h>

// Declare functions (these will be in a separate header file in larger programs)
void InitialiseGeneral();
void InitialiseTimer0();
void InitialiseTimer1();
void InitialiseTimer3();

volatile uint8_t count;

// Declare global variables
unsigned char timer0_cnt;
unsigned char timer0_interval;

unsigned char timer1_cnt;
unsigned char timer1_interval;

unsigned char timer3_cnt;
unsigned char timer3_interval;

int main(void)
{
    timer0_cnt = 0;   // Initialise the count to 0
    timer0_interval = 2;  // try with different values here
    timer1_cnt = 0;           
    timer1_interval = 2;    // try with different values here
    timer3_cnt = 0;
    timer3_interval = 2;    // try with different values here
    
    InitialiseGeneral();
    InitialiseTimer0();
    InitialiseTimer1();
    InitialiseTimer3();
    
    while(1) {}
}

void InitialiseGeneral()
{
    DDRK = 0xFF;    // Configure PortK direction for Output
    PORTK = 0x00;   // Set all LEDs initially off
    sei();  
}

/* - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */
void InitialiseTimer0()     // Configure to generate an interrupt after a 1/4 Second interval
{
    //** NB! Timer0 is 8 bits **
    
    TCCR0A = 0x00;          // Normal port operation (OC0A, OC0B), Normal waveform generation
    TCCR0B = 0b00000101;    // Normal waveform generation, Use 1024 prescaler
    
    // For 1 MHz clock (with 1024 prescaler) Overflow occurs after counting to 256 (but already divided by 1024)
    // So overflow occurs after 1024 * 256 / 1000000 = 0.26 seconds
    
    TCNT0 = 0x00;           // Timer/Counter count/value register
    TIMSK0 = 0b00000001;    // Use 'Overflow' Interrupt, i.e. generate an interrupt
    // when the timer reaches its maximum count value (TOIE0: Timer/Counter0 Overflow Interrupt Enable) 
}

ISR(TIMER0_OVF_vect) // TIMER0_Overflow_Handler (Interrupt Handler for Timer 0)
{
    PORTK &= ~(1 << PINK0); // LED0 low
    PORTK |= (1 << PINK1);  // LED1 high
    timer0_cnt++;
    if (timer0_cnt == timer0_interval)
    {
        PORTK &= ~(1 << PINK1); // LED1 low
        PORTK |= (1 << PINK0);  // LED0 high
        timer0_cnt = 0;
    }
    TCNT0 = 0x00;
    
/*  
    clk_freq = 1,000,000/1024 = 977 Hz
    T = 1/clk_freq = 1/977 Hz = 1.024 ms
    Timer counter overflows 256 * 1.024 ms = 262 ms

    For 1/4 sec delay (256 ms)/(262 ms) = ~1
    The Timer0 counter has to overflow 1 time(s) to generate a 1/4 s delay.
    
    For 1 sec delay (1000 ms)/(262 ms) = 3.8 = ~4
    The Timer0 counter has to overflow 4 time(s) to generate a 1 s delay.
    
    For 4 sec delay (4000 ms)/(262 ms) = ~15
    The Timer0 counter has to overflow 15 time(s) to generate a 4 s delay.
*/

/*
    if (count >= 4) // used for a 4 sec delay
    {
        PORTK ^= (1 << PINK1);
        count = 0;
    }   
    else 
        count++;
*/
    
}

/* - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */
void InitialiseTimer1()     // Configure to generate an interrupt after a 2-Second interval
{
    TCCR1A = 0x00;  // Normal port operation (OC1A, OC1B, OC1C), Clear Timer on 'Compare Match' (CTC) waveform mode)
    TCCR1B = 0b00001101;    // CTC waveform mode, use prescaler 1024
    TCCR1C = 0x00;
    
    // For 1 MHz clock (with 1024 prescaler) to achieve a 2 second interval:
    // Need to count 2 million clock cycles (but already divided by 1024)
    // So actually need to count to (2000000 / 1024 =) 1953 decimal, = 7A1 Hex
    
    //OCR1AH = 0x07; // Output Compare Registers (16 bit)
    //OCR1AL = 0xA1;

    // 1 sec interval
    // So actually need to count to (1000000 / 1024) = 977 decimal = 3D1 Hex
    OCR1AH = 0x03; // Output Compare Registers (16 bit) OCR1BH and OCR1BL
    OCR1AL = 0xD1;

    TCNT1H = 0x00;  // Timer/Counter count/value registers (16 bit) TCNT1H and TCNT1L
    TCNT1L = 0x00;
    TIMSK1 = 0b00000010;    // bit 1 OCIE1A     Use 'Output Compare A Match' Interrupt, i.e. generate an interrupt
    // when the timer reaches the set value (in the OCR1A registers)
}

ISR(TIMER1_COMPA_vect) // TIMER1_CompareA_Handler (Interrupt Handler for Timer 1)
{
    PORTK &= ~(1 << PINK3);
    PORTK |= (1 << PINK4);
    timer1_cnt++;
        
    if (timer1_cnt == timer1_interval)
    {
        PORTK &= ~(1 << PINK4);
        PORTK |= (1 << PINK3);
        timer1_cnt = 0;
    }
    TCNT1H = 0x00;  // Timer/Counter count/value registers (16 bit) TCNT3H and TCNT3L
    TCNT1L = 0x00;
}

/* - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */
void InitialiseTimer3()     // Configure to generate an interrupt after a 0.5 Second interval
{
    TCCR3A = 0x00;  // Normal port operation (OC1A, OC1B, OC1C), Clear Timer on 'Compare Match' (CTC) waveform mode)
    TCCR3B = 0b00001101;    // CTC waveform mode, use prescaler 1024
    TCCR3C = 0x00;
    
    // For 1 MHz clock (with 1024 prescaler) to achieve a 0.5 second interval:
    // Need to count 500,000 clock cycles (but already divided by 1024)
    // So actually need to count to (500000 / 1024 =) 488 decimal, = 1E8 Hex
    OCR3AH = 0x01; // Output Compare Registers (16 bit) OCR1BH and OCR1BL
    OCR3AL = 0xE8;

    TCNT3H = 0x00;  // Timer/Counter count/value registers (16 bit) TCNT3H and TCNT3L
    TCNT3L = 0x00;
    TIMSK3 = 0b00000010;    // bit 1 OCIE3A     Use 'Output Compare A Match' Interrupt, i.e. generate an interrupt
    // when the timer reaches the set value (in the OCR3A registers)
}

ISR(TIMER3_COMPA_vect) // TIMER3_Overflow_Handler (Interrupt Handler for Timer 3)
{
    PORTK &= ~(1 << PINK6);
    PORTK |= (1 << PINK7);
    timer3_cnt++;
    if (timer3_cnt == timer3_interval)
    {
        PORTK &= ~(1 << PINK7);
        PORTK |= (1 << PINK6);  
        timer3_cnt = 0;
    }
    TCNT3H = 0x00;  // Timer/Counter count/value registers (16 bit) TCNT3H and TCNT3L
    TCNT3L = 0x00;
}
