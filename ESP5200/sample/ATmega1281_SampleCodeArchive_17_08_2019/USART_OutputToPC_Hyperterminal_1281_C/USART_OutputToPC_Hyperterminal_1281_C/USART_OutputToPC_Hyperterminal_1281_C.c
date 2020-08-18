//*******************************************************************************
// Project 		USART - Output messages via the serial interface to PC (Hyperterminal)
// Target 		ATMEL ATmega1281 micro-controller on STK300 board
// Program		USART_OutputToPC_Hyperterminal_1281_C.c
// Author		Richard Anthony
// Date			29th January 2013

// Function		Display diagnostic messages on PC using The Atmel's USART0 and using Hyperterminal on the PC
//				A serial-to-USB converter cable may be needed if the PC / Laptop does not have a 9-pin serial port.

// Communication setup of USART (Hyperterminal settings must match these)
//		Tx / Rx rate	Bits per second		9600
//		Data bits							   8
//		Parity								None
//		Stop bits							   1
//		Flow control						 H/W

// The ATmega1281 has two USARTs (numbered 0 and 1). This program uses USART 0
//					USART0 RxD is on Port E bit 0 (Input)
//					USART0 TxD is on Port E bit 1 (Output)

// FUSEs			1281 clock must be set at 1Mhz, otherwise UART clock-rate configuration must be adjusted
//*******************************************************************************

#include <avr/io.h>
#include <avr/interrupt.h>
#include <util/delay.h>

#define CR 0x0D
#define LF 0x0A
#define SPACE 0x20

void InitialiseGeneral();
void USART_SETUP_9600_BAUD_ASSUME_1MHz_CLOCK();
void USART_TX_SingleByte(unsigned char cByte) ;
void USART_DisplayBanner();

int main( void )
{
	unsigned char cDisplayChar = 'A';
	InitialiseGeneral();
	USART_SETUP_9600_BAUD_ASSUME_1MHz_CLOCK();

	USART_DisplayBanner();		// Display startup banner on Hyperterminal

	while(1)
    {
		USART_TX_SingleByte(cDisplayChar++);
		if('Z' < cDisplayChar)
		{
			cDisplayChar = 'A';
			USART_TX_SingleByte(LF);
			USART_TX_SingleByte(CR);
		}
		_delay_ms(1000);
	}
}

void InitialiseGeneral()
{
	sei();					// Enable interrupts at global level set Global Interrupt Enable (I) bit
}

void USART_SETUP_9600_BAUD_ASSUME_1MHz_CLOCK()
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
	UCSR0C = 0b00000111;		// Asynchronous, No Parity, 1 stop, 8-bit data, Falling XCK edge

// UBRR0 - USART Baud Rate Register (16-bit register, comprising UBRR0H and UBRR0L) 
	UBRR0H = 0; // 9600 baud, UBRR = 12, and  U2X must be set to '1' in UCSRA
	UBRR0L = 12;
}

ISR(USART0_RX_vect) // (USART_RX_Complete_Handler) USART Receive-Complete Interrupt Handler
{
	char cData = UDR0;
	USART_TX_SingleByte(cData); // Echo the received character
}

void USART_TX_SingleByte(unsigned char cByte) 
{
	while(!(UCSR0A & (1 << UDRE0)));	// Wait for Tx Buffer to become empty (check UDRE flag)
	UDR0 = cByte;	// Writing to the UDR transmit buffer causes the byte to be transmitted
}

void USART_DisplayBanner()
{
	int i;
	for(i = 0; i < 5; i++)
	{
		USART_TX_SingleByte(LF);	
	}
	USART_TX_SingleByte(CR);	
	for(i = 0; i < 20; i++)
	{
		USART_TX_SingleByte('*');	
	}
	USART_TX_SingleByte(LF);	
	USART_TX_SingleByte(CR);	
	for(i = 0; i < 5; i++)
	{
		USART_TX_SingleByte(SPACE);	
	}
	USART_TX_SingleByte('U');	
	USART_TX_SingleByte('S');
	USART_TX_SingleByte('A');	
	USART_TX_SingleByte('R');
	USART_TX_SingleByte('T');	
	USART_TX_SingleByte(SPACE);	
	USART_TX_SingleByte('D');
	USART_TX_SingleByte('E');	
	USART_TX_SingleByte('M');
	USART_TX_SingleByte('O');	
	USART_TX_SingleByte(LF);	
	USART_TX_SingleByte(CR);	
	for(i = 0; i < 20; i++)
	{
		USART_TX_SingleByte('*');	
	}
	USART_TX_SingleByte(LF);	
	USART_TX_SingleByte(CR);	
}