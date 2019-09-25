
#define F_CPU 3686000
#include <avr/io.h>
#include <util/delay.h>
#include <avr/interrupt.h>
#include <avr/wdt.h>

volatile unsigned char i = 0;
volatile unsigned char run = 1;
volatile unsigned int ovf_counter = 0;

ISR(INT0_vect) {
	i = 0;
}

ISR(INT1_vect) {
	run = 1-run;
}

ISR(TIMER0_OVF_vect) {
	TCNT0 = 72;
	if(run)	{
		if(ovf_counter == 19999) {	// 20.000 interrupts i sekundet
			ovf_counter = 0;
				i++;
		}
		else
			ovf_counter++;
	}
}

int main(void) {
	DDRC  = 0xFF;
	TCCR0 = 1;				// Timer/Counter Control Register
	PORTC = 0xFF;
	TIMSK =  0x01;			// TOIE0
	MCUCR = 0b1010;   		// ISC01 og ISC11
	GICR  = 0b11000000;		// INT1 og INT0
	sei();
	while(1) {
		PORTC =~ i;
	}
}
