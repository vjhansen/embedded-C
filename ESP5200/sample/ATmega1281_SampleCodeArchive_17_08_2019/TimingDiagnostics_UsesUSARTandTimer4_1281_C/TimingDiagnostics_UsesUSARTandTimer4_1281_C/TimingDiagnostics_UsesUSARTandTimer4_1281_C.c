//*******************************************************************************
// Project 		Timing Diagnostics uses USART to output timing diagnostic values via the serial interface to PC (Hyperterminal)
// Target 		ATMEL ATmega1281 micro-controller on STK300 board
// Program		TimingDiagnostics_UsesUSARTandTimer4_1281_C.c
// Author		Richard Anthony
// Date			13th October 2015

// Function		Record timing characteristics of program operation, using special checkpoint functions (which use Timer4)
//				Display Timing Diagnostic messages on PC using The Atmel's USART0 and using Hyperterminal on the PC
//				A serial-to-USB converter cable may be needed if the PC / Laptop does not have a 9-pin serial port.

//				Note: To demonstrate the concept, checkpoints are taken within the main loop which includes an approximately 1 second delay


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
#include <string.h>

#define CR 0x0D
#define LF 0x0A
#define SPACE 0x20
#define UC unsigned char
#define CheckPointArraySize (int)1000

void InitialiseGeneral();

// *** Start of TimingDiagnostic declarations ***
void Initialise_TimingDiagnostic();;
void ConfigureTimer4_for_1msPrecisionCheckPointing();
void TimingDiagnostic_ResetTimer4();
void TimingDiagnostic_CheckPoint(UC iCheckPointArrayIndex);
void TimingDiagnostic_Display_CheckPoint_DataViaUSART0(int iLimit_ArrayIndex);
void USART0_SETUP_9600_BAUD_ASSUME_1MHz_CLOCK();
void USART0_TX_SingleByte(unsigned char cByte);
void USART0_TX_String(char* sData);
void USART0_DisplayBanner_Head();
void USART0_DisplayBanner_Tail();
int g_iCheckpointArray[CheckPointArraySize];
UC g_iCheckPointArrayIndex; // Points to the next position to be written in the CheckPoint Array
// *** End of TimingDiagnostic declarations ***

UC cDisplayChar;

int main( void )
{
	int iMainLoopCount;
	cDisplayChar = 'A';

	InitialiseGeneral();
	Initialise_TimingDiagnostic();
	g_iCheckPointArrayIndex = 0;

	TimingDiagnostic_CheckPoint(g_iCheckPointArrayIndex++);	// Initial timing reference

	for(iMainLoopCount = 0; iMainLoopCount < 8; iMainLoopCount++)
	{
		USART0_TX_SingleByte(cDisplayChar++);
		TimingDiagnostic_CheckPoint(g_iCheckPointArrayIndex++);
		_delay_ms(1000);	// 1 second delay
		TimingDiagnostic_CheckPoint(g_iCheckPointArrayIndex++);
	}
	TimingDiagnostic_CheckPoint(g_iCheckPointArrayIndex++);	// Final timing reference
	TimingDiagnostic_Display_CheckPoint_DataViaUSART0(g_iCheckPointArrayIndex);
	while(1)	// End of program execution - loop here
	{
	}
}

void InitialiseGeneral()
{
	sei();					// Enable interrupts at global level set Global Interrupt Enable (I) bit
}

// *** The Timing Diagnostic support methods are defined from here onwards ***
void Initialise_TimingDiagnostic()
{
	ConfigureTimer4_for_1msPrecisionCheckPointing();
	USART0_SETUP_9600_BAUD_ASSUME_1MHz_CLOCK();
}

void ConfigureTimer4_for_1msPrecisionCheckPointing()	// Configure to generate an interrupt every 1 MilliSecond (actually every 1.024 mS)
														// Thus can time events up to 2^16 milliseconds = approximately 1000 seconds, with 1.024ms precision
{
	TCCR4A = 0b00000000;	// Normal port operation (OC4A, OC4B, OC4C), Normal waveform mode)
	TCCR4B = 0b00000101;	// Normal waveform mode, use prescaler 1024
	TCCR4C = 0b00000000;

	OCR4AH = 0x00; // Output Compare Registers (16 bit) OCR4AH and OCR4AL
	OCR4AL = 0x00;
	OCR4BH = 0x00; // Output Compare Registers (16 bit) OCR4BH and OCR4BL
	OCR4BL = 0x00;

	TCNT4H = 0x00;	// Timer/Counter count/value registers (16 bit) TCNT4H and TCNT4L
	TCNT4L = 0x00;
	TIMSK4 = 0b00000000;	// Not using interrupts
}

void TimingDiagnostic_ResetTimer4()	// Reset count to 0
{
	TCNT4H = 0x00;	// Timer/Counter count/value registers (16 bit) TCNT4H and TCNT4L
	TCNT4L = 0x00;
}

void TimingDiagnostic_CheckPoint(UC iCheckPointArrayIndex)	// Record a CheckPoint value
{
	int iCheckPoint_Value = (TCNT4H * 256) + TCNT4L;
	g_iCheckpointArray[iCheckPointArrayIndex] = iCheckPoint_Value;
}

void TimingDiagnostic_Display_CheckPoint_DataViaUSART0(int iLimit_ArrayIndex)	// Display CheckPoint values
		// Call this method at the end of execution to avoid disturbing timing whilst recording CheckPoints
{
	char cArrayIndex[10];
	char cArrayValue[10];
	char cDisplayString[100];
	long lMilliseconds = 0;	
	USART0_DisplayBanner_Head();
		
	for(int iCount = 0; iCount < iLimit_ArrayIndex; iCount++)
	{
		itoa(iCount, cArrayIndex, 10);
		itoa(g_iCheckpointArray[iCount], cArrayValue, 10);
		lMilliseconds = ((long)g_iCheckpointArray[iCount] * 1024) / 1000;
		sprintf(cDisplayString, "          %4s        %7s    %8d", cArrayIndex, cArrayValue, lMilliseconds);
		USART0_TX_String(cDisplayString);
	}
	USART0_DisplayBanner_Tail();
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
	UCSR0C = 0b00000111;		// Asynchronous, No Parity, 1 stop, 8-bit data, Falling XCK edge

// UBRR0 - USART Baud Rate Register (16-bit register, comprising UBRR0H and UBRR0L)
	UBRR0H = 0; // 9600 baud, UBRR = 12, and  U2X must be set to '1' in UCSRA
	UBRR0L = 12;
}

ISR(USART0_RX_vect) // (USART_RX_Complete_Handler) USART Receive-Complete Interrupt Handler
{
	char cData = UDR0;
	// USART0_TX_SingleByte(cData); // Implements echo, (not used)
}

void USART0_TX_SingleByte(unsigned char cByte)
{
	while(!(UCSR0A & (1 << UDRE0)));	// Wait for Tx Buffer to become empty (check UDRE flag)
	UDR0 = cByte;	// Writing to the UDR transmit buffer causes the byte to be transmitted
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

void USART0_DisplayBanner_Head()
{
	USART0_TX_String("\r\n\n*****************************************************");
	USART0_TX_String("      Timing diagnostics (Time unit is 1.024ms)");	
	USART0_TX_String("       CheckPoint no.   Value    Milliseconds");
}

void USART0_DisplayBanner_Tail()
{
	USART0_TX_String("*****************************************************");
}