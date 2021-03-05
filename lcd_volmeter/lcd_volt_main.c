/*
	Program acts as a voltmeter which reads voltages from 0 - 5000 mV on PORTA PIN 7.
	Measured voltages are shown on a LCD.
*/

#define F_CPU 3686400		// - CPU clk speeed

#include <avr/io.h>
#include <stdio.h>
#include <util/delay.h>	
#include <avr/interrupt.h>	
#include "LCD_lib.h"		

volatile unsigned int curr_val;		// - Global variable holding the voltage value
volatile unsigned int prev_val = 0;
volatile unsigned int potmeter_in;		// - Holding ADCW. 
volatile unsigned int max = 1;			
volatile unsigned int min = 0;

#define row1 0
#define row2 1

// ADC Conversion Complete Interrupt
ISR(ADC_vect) 
{
    potmeter_in = ADCW;
    curr_val = (((unsigned long)potmeter_in*5000)/1023);	    // - 1000x voltage
    ADCSRA = ADCSRA|0x40;	// - Start new conversion
}


int main() 
{
	PORTC  = 0xFF;		
	DDRC   = 0xF7;		// - PINC.3 pulled low via 1k ohm resistor
	ADMUX  = 0x07;		// - Sampling ch. 0
	ADCSRA = 0b11001101;	// - Init. ADC and start measuring
	
	sei();			// - Init. global interrupt

	initialize_display();
	
	while(1) 
	{
		if (prev_val != curr_val) 
		{
		    if (max < curr_val) 
		    {
				max = curr_val;
		    }
		    else if (min > curr_val) 
		    {
				min = curr_val;
		    }
		    prev_val = curr_val;

		    // Screen 2 (Max/Min)
		    if ((PINA & 0b0001) == 0) 
		    {
				LCD_clear();
				lcd_string(row1, 0, "MAX:");
				lcd_string(row1, 14, "mV");
				LCD_print_numb(0, 10, max);
				lcd_string(row2, 0, "MIN:");
				lcd_string(row2, 14, "mV");
				LCD_print_numb(row2, 10, min);
				_delay_ms(100);
		    }

			// Reset
		    else if ((PINA & 0b0010) == 0) 
		    {
				max = 1;
				min = 0;
			}

			// Screen 1 (Standard)
		    else if ((PINA & 0b0001) == 1) 
		    {
				LCD_clear();
				lcd_string(row1, 0, "ADC:");
				LCD_print_numb(row1, 10, potmeter_in);
				lcd_string(row2, 0, "Voltage:");
				lcd_string(row2, 14, "mV");
				LCD_print_numb(row2, 10, curr_val);
				_delay_ms(100);
		    }
		}
	}
}
