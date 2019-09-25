

/*

~*~*~*~ Oblig. 2 AUT-1001 ~*~*~*~

|| Victor Hansen, AU1 |||| Atle Egeland, AU1  ||

Levert: 29.03.2017

Program acts as a voltmeter which reads voltages from 0 - 5000 mV on PORTA PIN 7.
Measured voltages are shown on a LCD.
Programmet inkluderer 'min_lcd.h' som inneholder funksjoner for LCD-displayet.

*/


#define F_CPU 3686400				// - CPU klokkehastighet.
#include <avr/io.h>
#include <stdio.h>
#include <util/delay.h>				// - Bibliotek for _delay_ms.
#include <avr/interrupt.h>			// - Bibliotek for interrupts.
#include "min_lcd.h"				// - Bibliotek for LCD-displayet.

volatile unsigned int milli_volt;		// - Global variabel som holder verdien til spenningen.
volatile unsigned int potmeter_inn;		// - Global variabel for � holde ADCW.
volatile unsigned int max = 1;			// - Global variabel som setter startverdi for 'max'.
volatile unsigned int min = 0;			// - Global variabel som setter startverdi for 'min'.
#define rad1 0
#define rad2 1


// ADC Conversion Complete Interrupt.
ISR(ADC_vect) {
    potmeter_inn = ADCW;
    milli_volt = (((unsigned long)potmeter_inn*5000)/1023);	    // - Typecaster og regner ut 1000 x spenningen.
    ADCSRA = ADCSRA|0x40;					 // - Starter ny konvertering.
}


int main() {
PORTC  = 0xFF;			// - PORTC is output.
DDRC   = 0xF7;			// - PINC.3 pulled low via 1k ohm resistor
ADMUX  = 0x07;			// - Sampling ch. 0
ADCSRA = 0b11001101;	// - Init. ADC and start measuring
sei();				    // - Init. global interrupt
	
initialize_display();
volatile unsigned int old_millivolt = 0;

while(1){
	if (old_millivolt != milli_volt) {
            if (max < milli_volt) {
                max = milli_volt;
            }
            else if (min > milli_volt) {
                min = milli_volt;
            }
            old_millivolt = milli_volt;
							
            // Skjerm 2 (Max/Min)
            if ((PINA&0b0001) == 0) {
                LCD_clear();
                lcd_string(rad1,0,"MAX:");
                lcd_string(rad1,14,"mV");
                LCD_print_numb(0,10,max);
                lcd_string(rad2,0,"MIN:");
                lcd_string(rad2,14,"mV");
                LCD_print_numb(rad2,10,min);
                _delay_ms(100);
            }
						
		// Reset. *funker ikke*
            else if ((PINA&0b0010) == 0) {
                max = 0;
                min = 0;
                }
						
		// Skjerm 1 (Standard)
            else if ((PINA&0b0001) == 1) {
                LCD_clear();
                lcd_string(rad1,0,"ADC:");
                LCD_print_numb(rad1,10,potmeter_inn);
                lcd_string(rad2,0,"Voltage:");
                lcd_string(rad2,14,"mV");
                LCD_print_numb(rad2,10,milli_volt);
                _delay_ms(100);
            }
        }
    }
}
