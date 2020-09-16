/*
uC: ATmega644A.
IDP: Atmel Studio 7.
*/

#include <avr/io.h>
#include <stdio.h>
#include "LCD_driver.h"
#include <avr/interrupt.h>
#include <util/delay.h>

#define idle	   0
#define disp_kort  1
#define PC         2
#define linefeed   0x0A

// Array for mottatt/sendt data
volatile unsigned char mottatt_data[70];
volatile unsigned char sendt_data[70];
volatile unsigned char ADC_array[50];

// Array som holder sortert data
unsigned char A_vect[20];
unsigned char B_vect[20];
unsigned char C_vect[20];
unsigned char D_vect[20];

// Diverse variabler
volatile unsigned int  modus = 1;
volatile unsigned int  cnt = 1;
volatile unsigned int  byte = 0;
volatile unsigned char i = 0; 
volatile unsigned char tx_count = 0;
volatile unsigned char rx_count = 0;
unsigned int element;
unsigned int ADC_data;
unsigned int milli_volt;
unsigned int freq;

// Interrupt når sending av data er ferdig
ISR(USART0_TX_vect) 
{   
    if(sendt_data[cnt])
    	UDR0 = sendt_data[cnt++];
}

// Interrupt når mottak av data er ferdig
ISR(USART0_RX_vect) 
{
    if(mottatt_data[byte-2] == 'R' && mottatt_data[byte-1] == linefeed) {
        byte  = 0;
        modus = disp_kort;
    } else
	mottatt_data[byte++] = UDR0;
}

// Interrupt ADC 
ISR(ADC_vect) 
{
    ADC_data = ADCW; // - hold ADCW value
    milli_volt = (((unsigned long)ADC_data * 5000)/1023);   // - Regner ut 1000x spenningen
    ADCSRA = ADCSRA|0x40; //0b 01000000   // - Starter ny konvertering
}

// Interrupt Timer2 
ISR(TIMER2_OVF_vect) 
{
    OCR2A = OCR2A;
}
 
int main(void) 
{
	DDRB   = 0b11110000;
	DDRC   = 0b11110000;
	DDRD   = 0b00000010;	// - Output PD1(TXD)
	PORTB  = 0b00001111;
	PORTC  = 0b00001111;

	/***********************/
	UCSR0A = 0b00000000;	// - Init. USART
	UCSR0B = 0b11011000;	// - RXCIE/TXCIE/RXEN/TXEN
	UBRR0  = 0b01011111;	// - Baudrate = 9600bps
	ADMUX  = 0b00000000;	// - ADC Multiplexer Selection Register
	TCCR2A = 0b01000011;	// - COM2A0/WGM21/WGM20
	TCCR2B = 0b00001100;	// - WGM22/CS22
	OCR2A  = 230;		// - Output Compare Register A
	ADCSRA = 0b11001111;	// - Init. ADC
	/***********************/

	sei();			// - Init. global interrupt
	
	init_lcd ();		// - Init. display
	modus = idle;
	_delay_ms(50);
  
while (1) {
	
    /**** do nothing ****/
    while(modus == idle) 
    {
    }
    /**** Sender data til displaykort ****/
    while(modus == disp_kort) 
    {
		 
    // A) - Behandler data for tekstlinje 1
        if(mottatt_data[rx_count] = 'A') {
            i = 0;
            rx_count++;
            while(mottatt_data[rx_count] != linefeed) {
                A_vect[i++] = mottatt_data[rx_count++];
            }
            rx_count++;
        }
    //- A) - Printer tekst (linje 1)
        lcd_printline(0,0,A_vect);
        
    //- B) - Behandler data for tekstlinje 2
        if(mottatt_data[rx_count] = 'B') {
            i = 0;
            rx_count++;
            while(mottatt_data[rx_count] != linefeed) {
                B_vect[i++] = mottatt_data[rx_count++];
            }
            rx_count++;
        }
		
    //- B) - Printer klokkeslett (linje 2)
        lcd_printline(1,0,B_vect);
        _delay_ms(100);
		 
    //- C) - Behandler data for LEDs
        if(mottatt_data[rx_count] = 'C') {
            i = 0;
            rx_count++;
            while(mottatt_data[rx_count] != linefeed) {
                C_vect[i++] = mottatt_data[rx_count++];
            }
            rx_count++;
        }
		 
    // LED 1 (AV/PÅ)
    if(C_vect[0] == '0') 
	    PORTB&=0b01111111;
    else 		 
	    PORTB|=0b10000000;

    // LED 2 (AV/PÅ)
    if(C_vect[1] == '0') 
	    PORTB&=0b10111111;
    else 		 
	    PORTB|=0b01000000;
		
    // LED 3 (AV/PÅ)
    if(C_vect[2] == '0') 
	    PORTB&=0b11011111;
    else	         
	    PORTB|=0b00100000;
		
    // LED 4 (AV/PÅ)
    if(C_vect[3] == '0') 
	    PORTB&=0b11101111;
    else 		 
	    PORTB|=0b00010000;

    // D) - Behandler data for piezo-element
    if(mottatt_data[rx_count] = 'D') {
        i = 0;
        rx_count++;
        while(mottatt_data[rx_count] != linefeed) {
            D_vect[i++] = mottatt_data[rx_count++];
        }
        rx_count++;
    }
    rx_count++;
    
    // D) - Frekvens for piezo-element
    if(D_vect[0] == '1') {
        DDRD |= 0b10000000;
        freq = ((D_vect[1] - '0')*1000) + ((D_vect[2] - '0')*100)
            +((D_vect[3] - '0')*10) + (D_vect[4]  - '0');
        OCR2A = freq;
    }
    else if(D_vect[0] == '0') {
        DDRD &=~0b10000000;
    }
    // Bytter nå modus..
    modus = PC;
}
	
/**** Sender data til program på PC ****/
while(modus == PC) 
{
	while(sendt_data[tx_count]) {
        	sendt_data[tx_count++] = 0;
    	}
    	tx_count = 0;
		
    	// A) - ADC
    	itoa(milli_volt, ADC_array, 10);	// - Printer ut spenning i mV
    	sendt_data[0]= 'A';
    	for(element = 0; element < 4; element++) {
        sendt_data[++tx_count] = ADC_array[element];
    }
    	sendt_data[++tx_count] = linefeed;
		
	// B) - Fritekst
    	volatile unsigned char fri_tekst[]= "1 Protokoll";		// - Her endres friteksten
    	sendt_data[++tx_count] = 'B';
    	element = 0;
    	tx_count++;
    	while(fri_tekst[element] && element < 17) {
        	sendt_data[tx_count++] = fri_tekst[element++];
	}
	sendt_data[tx_count++] = linefeed;
			
	// C) - DIP-brytere
	sendt_data[tx_count++] = 'C';
	// Bryter 1
	if((PINB & (1<<0)))		
        	sendt_data[tx_count++] = '0';	
	else					
        	sendt_data[tx_count++] = '1';
		
	// Bryter 2
	if((PINB & (1<<1)))     
        	sendt_data[tx_count++] = '0';	
	else
        	sendt_data[tx_count++] = '1';
		
	// Bryter 3
	if((PINB & (1<<2)))		
        	sendt_data[tx_count++] = '0';	
	else					
        	sendt_data[tx_count++] = '1';
		
	// Bryter 4
	if((PINB & (1<<3)))		
        	sendt_data[tx_count++] = '0';	
	else					
        	sendt_data[tx_count++] = '1';
		sendt_data[tx_count++] = linefeed;
		
	// D) - Adressebryter
	sendt_data[tx_count++] = 'D';
	unsigned char bryter = (PINC&0b00001111);
	sprintf(&sendt_data[tx_count++],"%x",bryter); 	// -  ASCII til HEX "%x"	
	sendt_data[tx_count++] = linefeed;
	_delay_ms(40);
	UDR0  = sendt_data[0];
	cnt   = 1;
	modus = idle;
	}	
	}
}
