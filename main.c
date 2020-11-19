
#include <avr/io.h>
#include <avr/interrupt.h>

// clock  8000000 MHz

volatile unsigned char uLED_value;
int main(void)
{ 
    //DDRD  = (1<<PD7 | 1<<PD6);   // PORTD = output
    DDRH  = (1<<PH4 | 1<<PH3);   // PORTD = output

    // PORTD = 0x00;
    PORTH = 0x00;
    TCCR1A = 0x00;    // Normal
    TCCR1B = (1<<WGM12 | 1<<CS12 | 1<<CS10);   //Clear Timer on Compare Match (CTC), 1024 prescaler
    TCCR1C = 0x00;
    // For 1 sec we need to count to ( (1 sec * 8000000 MHz) / 1024) = 0x1E84
    // what if clk = 16 MHz? -> 3D09
    OCR1AH = 0x3D; // Output Compare Registers (16 bit)
    OCR1AL = 0x09;
    
    TCNT1H = 0x00;  // Timer/Counter count/value registers (16 bit)
    TCNT1L = 0x00;
    TIMSK1 = (1<<OCIE1A);
    uLED_value =0x00;
   
    sei();      // Enable global interrupt
    
    while (1) {}
}


ISR(TIMER1_COMPA_vect) { // TIMER1_CompareA_Handler (Interrupt Handler for Timer 1)
      // Flip the value of the least significant bit of the 8-bit variable
      if(0 == uLED_value) {
        uLED_value = 1;
        //PORTD = 0b10000000;
        PORTH = 0b0001000;
        
      }
      else {
        uLED_value = 0;
        // PORTD = 0b01000000;
         PORTH = 0b00010000;
        
      }
}
