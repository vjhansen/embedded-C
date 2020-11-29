//*******************************************************************************
// Project      USART serial interface to PC (Hyperterminal) - simple demonstration
// Target       ATMEL ATmega2560 micro-controller on Arduino Mega board
// Program      USART_2560_C.c
// Author       Richard Anthony
// Date         21th October 2015

// Function     Simple demonstration of the use of the USART to interface to an external system (e.g. a PC running Hyperterminal or other terminal emulator)

// Communication setup of USART (Hyperterminal settings must match these)
//      Tx / Rx rate    Bits per second     9600
//      Data bits                              8
//      Parity                              None
//      Stop bits                              1
//      Flow control                         H/W

// The ATmega2560 has four USARTs (numbered 0 - 3). This program uses USART 0
//                  USART0 RxD is on Port E bit 0 (Input)
//                  USART0 TxD is on Port E bit 1 (Output)

// FUSEs            2560 clock must be set at 1Mhz, otherwise UART clock-rate configuration must be adjusted
//*******************************************************************************

#include <avr/io.h>
#include <avr/interrupt.h>

#define F_CPU 1000000UL

#include <util/delay.h>
#include <string.h>

#define CR 0x0D // Carriage return (New line)
#define LF 0x0A // Line feed
#define SPACE 0x20

void InitialiseGeneral();
void USART0_SETUP_9600_BAUD_ASSUME_1MHz_CLOCK();
void USART0_TX_SingleByte(unsigned char cByte);
void USART0_TX_String(char* sData);
void USART0_DisplayBanner();
void USART0_DisplayPrompt();

volatile unsigned char cMyData; //declare data types as volatile if they are global or gets interrupted by timer
volatile unsigned char flag_dir; // set direction
volatile unsigned char flag_speed; // set delay
volatile unsigned char flag_pause; // start/stop


int main(void)
{
    USART0_SETUP_9600_BAUD_ASSUME_1MHz_CLOCK();
    InitialiseGeneral();
    USART0_DisplayBanner();

    while(1) 
    {           
        while (flag_pause == 's' && flag_pause != 'p') {
        
            // loop Forward
            if(flag_dir == 'f')
            {
                if( cMyData < 65 || cMyData > 90)
                {
                    cMyData = 65;
                    USART0_TX_SingleByte(LF);
                    USART0_TX_SingleByte(CR);
                }
                if (flag_speed == '+')
                {
                    USART0_TX_SingleByte(cMyData++);
                    _delay_ms(250);
                }           
                if (flag_speed == '-')
                {
                    USART0_TX_SingleByte(cMyData++);
                    _delay_ms(1000);
                }                   
            }

            // loop backward
            if(flag_dir == 'b')
            {
                if( cMyData < 65 || cMyData > 90)
                {
                    cMyData = 90;
                    USART0_TX_SingleByte(LF);
                    USART0_TX_SingleByte(CR);
                }
                else if (flag_speed == '+')
                {
                    USART0_TX_SingleByte(cMyData--);
                    _delay_ms(250);
                }
                else if (flag_speed == '-')
                {
                    USART0_TX_SingleByte(cMyData--);
                    _delay_ms(1000);
                }
            }
        }
    }
}

void InitialiseGeneral()
{
    sei();      // Enable interrupts at global level set Global Interrupt Enable (I) bit
}

void USART0_SETUP_9600_BAUD_ASSUME_1MHz_CLOCK()
{
//UCSR0A – USART Control and Status Register A
    // bit 7 RXC Receive Complete (flag)
    // bit 6 TXC Transmit Complete (flag)
    // bit 5 UDRE Data Register Empty (flag)
    // bit 4 FE Frame Error (flag) - programmatically clear this when writing to UCSRA
    // bit 3 DOR Data OverRun (flag)
    // bit 2 PE Parity Error
    // bit 1 UX2 Double the USART TX speed (but also depends on value loaded into the Baud Rate Registers)
    // bit 0 MPCM Multi-Processor Communication Mode
    UCSR0A = 0b00000010; // Set U2X (Double the USART Tx speed, to reduce clocking error)

// UCSR0B - USART Control and Status Register B
    // bit 7 RXCIE Receive Complete Interrupt Enable
    // bit 6 TXCIE Transmit Complete Interrupt Enable
    // bit 5 UDRIE Data Register Empty Interrupt Enable
    // bit 4 RXEN Receiver Enable
    // bit 3 TXEN Transmitter Enable
    // bit 2 UCSZ2 Character Size (see also UCSZ1:0 in UCSRC)
    // 0 = 5,6,7 or 8-bit data
    // 1 = 9-bit data
    // bit 1 RXB8 RX Data bit 8 (only for 9-bit data)
    // bit 0 TXB8 TX Data bit 8 (only for 9-bit data)
    UCSR0B = 0b10011000;  // RX Complete Int Enable, RX Enable, TX Enable, 8-bit data

// UCSR0C - USART Control and Status Register C
// *** This register shares the same I/O location as UBRRH ***
    // Bits 7:6 – UMSELn1:0 USART Mode Select (00 = Asynchronous)
    // bit 5:4 UPM1:0 Parity Mode
    // 00 Disabled
    // 10 Even parity
    // 11 Odd parity
    // bit 3 USBS Stop Bit Select
    // 0 = 1 stop bit
    // 1 = 2 stop bits
    // bit 2:1 UCSZ1:0 Character Size (see also UCSZ2 in UCSRB)
    // 00 = 5-bit data (UCSZ2 = 0)
    // 01 = 6-bit data (UCSZ2 = 0)
    // 10 = 7-bit data (UCSZ2 = 0)
    // 11 = 8-bit data (UCSZ2 = 0)
    // 11 = 9-bit data (UCSZ2 = 1)
    // bit 0 UCPOL Clock POLarity
    // 0 Rising XCK edge
    // 1 Falling XCK edge
    UCSR0C = 0b00000111;        // Asynchronous, No Parity, 1 stop, 8-bit data, Falling XCK edge

// UBRR0 - USART Baud Rate Register (16-bit register, comprising UBRR0H and UBRR0L)
    UBRR0H = 0; // 9600 baud, UBRR = 12, and  U2X must be set to '1' in UCSRA
    //UBRR0L = 12;
    // UBRR0L = 103;
    UBRR0L = 207;
}

ISR(USART0_RX_vect) // (USART_RX_Complete_Handler) USART Receive-Complete Interrupt Handler
{
    char cData = UDR0;
    switch(cData)
    {
        case 'f': // loop forward
            flag_dir = 'f'; 
            break;
        case 'b': // loop backward
            flag_dir = 'b';
            break;
        case '+': // shorter delay
            flag_speed = '+';
            break;
        case '-': // longer delay
            flag_speed = '-';
            break;
        case 'p': // stop
            flag_pause = 'p';
            break;
        case 's': // start
            flag_pause = 's';
            break;
    }
    USART0_DisplayPrompt();
}

void USART0_TX_SingleByte(unsigned char cByte)
{
    while( !(UCSR0A & (1 << UDRE0)) );  // Wait for Tx Buffer to become empty (check UDRE flag)
    UDR0 = cByte;   // Writing to the UDR transmit buffer causes the byte to be transmitted
}

void USART0_TX_String(char* sData)
{
    int iCount;
    int iStrlen = strlen(sData);
    if(0 != iStrlen) {
        for(iCount = 0; iCount < iStrlen; iCount++)
        {
            USART0_TX_SingleByte(sData[iCount]);
        }
        USART0_TX_SingleByte(CR);
        USART0_TX_SingleByte(LF);
    }
}


void USART0_DisplayBanner()
{
    USART0_TX_String("\r\n\n*****************************************************");
    USART0_TX_String("  Demo - Alphabet loop");
    USART0_TX_String("  Instructions for usage:"); 
    USART0_TX_String("      1. Press 's' first, in order to start the counter"); 
    USART0_TX_String("      2. Press 'f' or 'b' to loop forward or backwards");
    USART0_TX_String("      3. Press '+' or '-' in order to select the speed"); 
    USART0_TX_String("*****************************************************");
    USART0_DisplayPrompt();
}

void USART0_DisplayPrompt()
{
    USART0_TX_String("Press 'f' = Forward/ 'b' = Backward/ '+' = Faster/ '-' = Slower/ 's' = Start/ 'p' = Pause");
}
