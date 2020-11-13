
#include <avr/io.h>
#include <avr/interrupt.h>

#include <util/delay.h>
//#define F_CPU 1000000UL

int main(void)
{ 
    DDRH  |= (1<<PH4 | 1<<PH3);   // PORTD = output
    PORTH = 0x00;
    TCCR1A = 0x00;    // Normal
    TCCR1B = (1<<WGM12 | 1<<CS12 | 1<<CS10);   //Clear Timer on Compare Match (CTC), 1024 prescaler
    TCCR1C = 0x00;
    // For 1 sec we need to count to ( (1 sec * 1000000 MHz) / 1024) = 0x03D0
    OCR1AH = 0x4c; // Output Compare Registers (16 bit)
    OCR1AL = 0x4b;
    
    TCNT1H = 0x00;  // Timer/Counter count/value registers (16 bit)
    TCNT1L = 0x00;
    TIMSK1 = (1<<OCIE1A);  
   
    sei();      // Enable global interrupt
    
    while (1)
    {
        _delay_ms(10000);
        PORTH = 0b00001000; 
        _delay_ms(10000);
        PORTH = 0b00010000;
    }
}

// something wrong here
ISR(TIMER1_COMPA_vect) 
{   
    // PORTH ^= (1<<PH4 | 1<<PH3);
}
