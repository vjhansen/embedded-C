
#define F_CPU 3686400
#include <avr/io.h>
#include <util/delay.h>
#include <avr/interrupt.h>
#define freq (460000/klokke)  // freq er målt frekvens = 460000 = 3680000/8

unsigned int ov_counter;
unsigned int starting_edge = 0;
unsigned int ending_edge = 0;
unsigned int klokke = 0;

// - teller overflow på timer1
ISR(TIMER1_OVF_vect) {	
	++ov_counter;
}

ISR(TIMER1_CAPT_vect) { 	
	ending_edge = ICR1L;
	ending_edge = (256*ICR1H)+ending_edge;
	klokke = (unsigned long)(ending_edge+(ov_counter*0xFFFF)-starting_edge);
	starting_edge = ending_edge;
	ov_counter = 0;
}
	
int main(void) {	
	DDRC   = 0xFF;		// PORTC = output
	TCCR1A = 0;			// Funksjon for komparatorer (off)
	TCCR1B = 0xC2;		// Funksjon for capture og prescaler
	TIMSK  = 0x24;		// Interrupts på capture og overflow
	sei();				// Enable globalt interrupt
    
    while (1) {
			
			if (freq<400)
			PORTC = ~0b00000000;
			
			else if(freq<600)		//0,4 - 0,6 kHz
			PORTC = ~0b00000001;
			
			else if(freq<700)		//0,6 - 0,7 kHz
			PORTC = ~0b00000010;
			
			else if(freq<800)		//0,7 - 0,8 kHz
			PORTC = ~0b00000100;
			
			else if(freq<900)		//0,8 - 0,9 kHz
			PORTC = ~0b00001000;
			
			else if(freq<1000)		//0,9 - 1,0 kHz
			PORTC = ~0b00010000;
			
			else if(freq<1100)		//1,0 - 1,1 kHz
			PORTC = ~0b00100000;
			
			else if(freq<1200)		//1,1 - 1,2 kHz
			PORTC = ~0b01000000;
			
			//else if(freq<1400)		//1,2 - 1,4 kHz
			//PORTC = ~0b10000000;
			
			else PORTC = ~0b00000000;	//Lys av
    };
}
