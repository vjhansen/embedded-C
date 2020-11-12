
#include <avr/io.h>
#include <avr/interrupt.h>

#define F_CPU 1000000UL
#include <util/delay.h>

	
int main(void)
{	
    DDRB  |= (1<<PB1 | 1<<PB0);		// PORTD = output
    PORTB = 0x00;
    
    TCCR1A = 0x00;		// Normal
    TCCR1B = (1<<CS12 | 1<<CS10);		// 1024 prescaler

         // For 1 sec we need to count to ( (1 sec * 1000000 MHz) / 1024) = 0x03D0
    OCR1AH = 0x03; // Output Compare Registers (16 bit)
    OCR1AL = 0xD0;
    
    TCNT1H = 0x00;  // Timer/Counter count/value registers (16 bit)
    TCNT1L = 0x00;
    TIMSK1 = (1<<OCIE1A);  
   
    sei();			// Enable global interrupt
    
    while (1) {		}
}



ISR(TIMER1_COMPA_vect) 
{ 	
    PORTB ^= (1<<PB1 | 1<<PB0);
}
