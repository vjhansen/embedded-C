
#define F_CPU 3686400
#include <avr/io.h>
#include <util/delay.h>				
#include "display.h"    // Headerfil for led-display - 'ledcode'
#include <avr/interrupt.h>			

volatile unsigned int ov_counter;       // Teller overflows
volatile unsigned int nybil = 0;	
volatile int strekning = 1000;
volatile unsigned int tid;		
volatile unsigned int fart;
volatile unsigned int S3;

ISR (TIMER1_OVF_vect) {   // Interrupt Timer1 Overflow
	ov_counter++;   // Icrement overflow counter
}

ISR (INT0_vect) { // Eksternt interrupt INT0(PD2)
	nybil = 1;      // Ny bil på vei
	fart = 0;       // Nullstiller fart
}

ISR (TIMER1_CAPT_vect) {
	if(nybil == 1) {
	        TCNT1 = 0;      // Nullstiller Timer1
		ov_counter = 0; // Nullstiller overflow counter
		nybil = 2;
	}
	else if (nybil == 2) {
		S3  = TCNT1L;           // Leser fra L-reg
		S3 += TCNT1H*256;       // Leser fra H-reg
		tid = ((unsigned long) S3 + (unsigned long) ov_counter * 65536)/461;
		nybil = 3;		// Stopper måling
		fart = strekning/tid;	// Regner ut fart
	}
}

int main(void) {
	DDRA = 0xFF;    // Output PORTA
	DDRB = 0xFF;	// Output PORTB
	DDRD = 0x00;	// Input  PORTC
	
	TCCR1B = 0xC2;		// Set ICNC1, ICES1 & CS11
	TIMSK  = 0b00100100; 	// Set TICIE1 & TOIE1
	MCUCR  = 0b00000010;	// ISC01
	GICR   = 0b01000000;	// INT0 
	
	sei();			// Enable interrupts
	
	while (1) {	
		PORTA = ledcode(fart/10)+1;     // Tall #1 og desimal
		PORTB = ledcode(fart%10);       // Tall #2
	}
}
