/*
  usart_2560.h
  
  Updated: Victor Hansen, 06.11.2020
  
  Based on:
  USART_2560_C (Richard Anthony, 21th October 2015)
  - - - - - - - - - - - - - - - - -
  
  Communication setup of USART:
  * Bits per second = 9600
  * Data bits       = 8
  * Parity          = None
  * Stop bits       = 1
  * Flow control    = H/W
  
  */


#include <avr/io.h>

#define CR  0x0D

void USART0_SETUP_9600_BAUD();
void USART0_TX_SingleByte(unsigned char cByte);
void USART0_TX_String(char* sData);

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
    USART0_TX_String("(P) enter new passcode on keypad / (D) distance / (Q) quit:\r\n");
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
