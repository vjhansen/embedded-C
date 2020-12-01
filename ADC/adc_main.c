#include <avr/io.h>
#include <avr/interrupt.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <math.h>

volatile unsigned char sample_flag = 1;
volatile unsigned int analog_temp = 0; 
volatile unsigned char hyperText[32];

#define CR  0x0D
#define LF  0x0A // Line feed
#define SPACE 0x20


void USART0_SETUP_9600_BAUD();
void USART0_TX_SingleByte(unsigned char cByte);
void USART0_TX_String(char* sData);
void Start_ADC_Conversion(void);
void init_timer1();
void init_adc();


int main()
{
    DDRH = (1<<PH4 | 1<<PH3);
    PORTH = 0x00;
    init_adc();
    init_timer1();
    USART0_SETUP_9600_BAUD();
    asm("sei");

    // Start the ADC Conversion (start first sample, runs in 'free run' mode after)
    //  bit 6 ADSC (ADC Start Conversion) = 1 (START)
    //  Read ADSCSR and OR with this value to set the flag without changing others
    
    ADCSRA |= 1<<ADSC; 
    while(1)
    {
        /* if (sample_flag == 1) { */

        /*     T = (float)analog_temp / THERMISTORNOMINAL;  */
        /*     TCNT1H = 0x00; */
        /*     TCNT1L = 0x00;  */
        /*     sample_flag = 0; */
        /* }    */
    }
}


void init_adc ()
{
    // ADC Multiplexer Selection Register
    ADMUX = (1<<REFS0 | 1<< ADLAR ); // AVCC with external capacitor at AREF pin,  ADC0 = PF0

    // ADLAR: ADC Left Adjust Result
    //  0 = ADCH (high) contains bit 1 = output bit 9, bit 0 = output bit 8
    //    ADCL (low) contains output bits 7 through 0
    //  1 = ADCH (high) contains bits 9 through 2
    //      ADCL (low) contains bit 7 = output bit 1, bit 6 = output bit 0
    // see datasheet p. 286

    
    ADCSRA = (1<<ADEN | 1<<ADIE | 1<<ADPS2 | 1<<ADPS1 | 1<<ADPS0);
    // ADC Enable, start conversion, auto trigger, division factor = 128.
    // we have  a 16 MHz clk, the ADC requires a clk freq. in the range [50, 200] kHz.
    // -> 16M/200k = 80, the next highest division factor is 128.
    
   
    // When this bit is written to one, Auto Triggering of the ADC is enabled. The ADC will start a conversion on a positive edge of the selected trigger signal. 
    ADCSRB = 0x00;

    // Digital Input Disable Register 
    DIDR0 = (1<<ADC0D); // disable digital input on the pin used for analog readings.
    DIDR1 = 0x00;
}

ISR(ADC_vect)
{
    // analog_temp = ADCL; // right shift
    analog_temp = ADCH;
    sprintf(hyperText, "%d", analog_temp);

    ADCSRA |= 1<<ADSC;
}

 
 ISR(USART0_RX_vect) // USART Receive-Complete Interrupt Handler
{
    char cData = UDR0;
    if (cData == 'p')
    {
        USART0_TX_String("\nTemp: \r\n");
        USART0_TX_String(hyperText);
    }
    else if (cData == 'q')
    {
        USART0_TX_String("\n q \r\n");
    }
}

// screen /dev/cu.usbserial 9600
// press Ctrl+a, type :quit and press Enter. 

void USART0_SETUP_9600_BAUD()
{
    // USART Control and Status Register A
    UCSR0A = (1<<U2X0); // Double the USART Transmission Speed
    // Writing this bit to one will reduce the divisor of the baud rate divider from 16 to 8 effectively doubling the transfer
    //rate for asynchronous communication.

    // USART Control and Status Register B
    /* RX Complete Interrupt Enable, RX Enable, TX Enable, 8-bit data */
    UCSR0B = (1<<RXCIE0 | 1<<RXEN0 | 1<<TXEN0);
    
    // USART Control and Status Register C
    /* Asynchronous, No Parity, 1 stop, 8-bit data, Falling XCK edge */
    UCSR0C = (1<<UCSZ01 | 1<<UCSZ00 | 1<<UCPOL0);
  
    // UBRR0 - USART Baud Rate Register (16-bit register, comprising UBRR0H and UBRR0L)
    UBRR0H = 0; // 9600 baud, UBRR = 12, and  U2X must be set to '1' in UCSRA
    UBRR0L = 207;
    // see datasheet p. 225, f_osc = 16 MHz
}

void USART0_TX_SingleByte(unsigned char cByte)
{
    while( !(UCSR0A & (1 << UDRE0)) ); // Wait for Tx Buffer to become empty (check UDRE flag)
    UDR0 = cByte;   // Writing to the UDR transmit buffer causes the byte to be transmitted
}

void USART0_TX_String(char* sData)
{
    int iCount;
    int iStrlen = strlen(sData);
    if(0 != iStrlen)
    {
        for(iCount = 0; iCount < iStrlen; iCount++)
        {
            USART0_TX_SingleByte(sData[iCount]);
        }
        USART0_TX_SingleByte(CR);
        USART0_TX_SingleByte(LF);
    }
}


void init_timer1()
{
    TCCR1A = 0x00;  // Normal port operation (OC1A, OC1B, OC1C), Clear Timer on 'Compare Match' (CTC) waveform mode)
    TCCR1B = (1<<WGM12 | 1<<CS12 | 1<<CS10);  // CTC waveform mode, initially stopped (no clock), prescaler = 1024
    // For 1 sec we need to count to ( (1 sec * 16 000 000 Hz) / 1024) = 0x3D09
    OCR1AH = 0x3D; // Output Compare Registers (16 bit)
    OCR1AL = 0x09;

    TCNT1H = 0x00;  // Timer/Counter count/value registers (16 bit)
    TCNT1L = 0x00;
    TIMSK1 = (1<<OCIE1A);  // generate an interrupt when the timer reaches the set value in the OCR1A register
}

ISR(TIMER1_COMPA_vect)
{
    sample_flag = 1;  
}
