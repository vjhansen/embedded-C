#include <avr/io.h>
#include <avr/interrupt.h>


// clock  8000000 MHz !!!!!!

volatile int analog_temp = 0; 
volatile unsigned char hyperText[32];


#define CR  0x0D

void USART0_SETUP_9600_BAUD();
void USART0_TX_SingleByte(unsigned char cByte);
void USART0_TX_String(char* sData);

void init_adc();

int main()
{

    DDRH = (1<<PH4 | 1<<PH3);
    PORTH = 0x00;
    init_adc();
    USART0_SETUP_9600_BAUD();
    asm("sei");


    // Start the ADC Conversion (start first sample, runs in 'free run' mode after)
    //  bit 6 ADSC (ADC Start Conversion) = 1 (START)
    //  Read ADSCSR and OR with this value to set the flag without changing others
    
    ADCSRA = ADCSRA | 0b01000000;
    
    while(1)
    {
           USART0_TX_SingleByte(unsigned char cByte)       
    }
}



void init_adc ()
{
    // ADC Multiplexer Selection Register
    ADMUX = (1<<REFS0); // AVCC with external capacitor at AREF pin,  ADC0 = PF0
    
    ADCSRA = (1<<ADEN | 1<<ADSC | 1<<ADIE | 1<<ADPS2 | 1<<ADPS0); // ADC Enable, start conversion, division factor = 32.
    ADCSRB = 0x00;

    // Digital Input Disable Register 
    DIDR0 = (1<<ADC0D); // disable digital input on the pin used for analog readings.
    DIDR1 = 0x00;
}

ISR(ADC_vect)
{
    analog_temp = ADCL;
    sprintf(hyperText, "%d", analog_temp);
    
}


/* 
 ISR(USART0_RX_vect) // USART Receive-Complete Interrupt Handler
{
    char cData = UDR0;
    // set new passcode
    if (cData == 'p')
    {
        USART0_TX_String("\nSet new passcode:\r\n");

    }
    // done setting passcode
    else if (cData == 'q')
    {
        USART0_TX_String("\nPasscode set\r\n");

        _delay_ms(500);
    }
   
}*/

// screen /dev/cu.usbserial 9600

void USART0_SETUP_9600_BAUD()
{
    // USART Control and Status Register A
    UCSR0A = (1<<U2X0); // Double the USART Transmission Speed
    
    // USART Control and Status Register B
    /* RX Complete Interrupt Enable, RX Enable, TX Enable, 8-bit data */
    UCSR0B = (1<<RXCIE0 | 1<<RXEN0 | 1<<TXEN0);
    
 
    // USART Control and Status Register C
    /* Asynchronous, No Parity, 1 stop, 8-bit data, Falling XCK edge */
    UCSR0C = (1<<UCSZ01 | 1<<UCSZ00 | 1<<UCPOL0);
  
    // UBRR0 - USART Baud Rate Register (16-bit register, comprising UBRR0H and UBRR0L)
    UBRR0H = 0; // 9600 baud, UBRR = 12, and  U2X must be set to '1' in UCSRA
    UBRR0L = 12;
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
    }
}
