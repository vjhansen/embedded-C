//*******************************************************************************
// Project 		Configure ZigBee Coordinator via USART1 (PortD, header pins)
// Target 		ATMEL ATmega1281 micro-controller on STK300 board
// Program		USART1_To_ZigBee_Coordinator_1281_C.c
// Author		Richard Anthony
// Date			2nd February 2014

// Notes		This sample code uses the second USART on the STK300 board - which connects via a two-pin header, and NOT via the 9-way D'Type connector
//				USART 1 is a special function on Port D bits 2 and 3

// Function		Display diagnostic messages on PC using The Atmel's USART1 and using Hyperterminal on the PC
//				A serial-to-USB converter cable may be needed if the PC / Laptop does not have a 9-pin serial port.

// Communication setup of USART (Hyperterminal settings must match these)
//		Tx / Rx rate	Bits per second		9600
//		Data bits							   8
//		Parity								None
//		Stop bits							   1
//		Flow control						 H/W

// The ATmega1281 has two USARTs (numbered 0 and 1). This program uses USART 1 (Port D)
//					USART0 RxD is on Port E bit 0 (Input)
//					USART0 TxD is on Port E bit 1 (Output)
//					USART1 RxD is on Port D bit 2 (Input)
//					USART1 TxD is on Port D bit 3 (Output)

// FUSEs			1281 clock must be set at 1Mhz, otherwise UART clock-rate configuration must be adjusted
//*******************************************************************************

#include <avr/io.h>
#include <avr/interrupt.h>
#include <util/delay.h>
#include <string.h>

#define CR 0x0D
#define LF 0x0A
#define SPACE 0x20

#define ZIGBEE_COMMAND_MAXIMUM_LENGTH 50
char ZigBee_AT_Command[ZIGBEE_COMMAND_MAXIMUM_LENGTH +1];


void InitialiseGeneral();
void USART1_SETUP_9600_BAUD_ASSUME_1MHz_CLOCK();
void USART1_TX_SingleByte(unsigned char cByte);
void USART1_TX_String(char* str);
void USART1_Configure_ZigBee_Coordinator();

int main( void )
{
	unsigned char cDisplayChar = 'A';
	InitialiseGeneral();
	USART1_SETUP_9600_BAUD_ASSUME_1MHz_CLOCK();

	USART1_Configure_ZigBee_Coordinator();		// Display startup banner on Hyperterminal

	while(1)
    {
		//USART1_TX_SingleByte(cDisplayChar++);
		//if('Z' < cDisplayChar)
		//{
			//cDisplayChar = 'A';
			//USART1_TX_SingleByte(LF);
			//USART1_TX_SingleByte(CR);
		//}
		//_delay_ms(1000);
	}
}

void InitialiseGeneral()
{
	sei();					// Enable interrupts at global level set Global Interrupt Enable (I) bit
}

void USART1_SETUP_9600_BAUD_ASSUME_1MHz_CLOCK()
{
//UCSR1A – USART Control and Status Register A
	// bit 7 RXC Receive Complete (flag)
	// bit 6 TXC Transmit Complete (flag)
	// bit 5 UDRE Data Register Empty (flag)
	// bit 4 FE Frame Error (flag) - programmatically clear this when writing to UCSRA
	// bit 3 DOR Data OverRun (flag)
	// bit 2 PE Parity Error
	// bit 1 UX2 Double the USART TX speed (but also depends on value loaded into the Baud Rate Registers)
	// bit 0 MPCM Multi-Processor Communication Mode
	UCSR1A = 0b00000010; // Set U2X (Double the USART Tx speed, to reduce clocking error)

// UCSR1B - USART Control and Status Register B
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
	UCSR1B = 0b10011000;  // RX Complete Int Enable, RX Enable, TX Enable, 8-bit data

// UCSR1C - USART Control and Status Register C
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
	UCSR1C = 0b00000111;		// Asynchronous, No Parity, 1 stop, 8-bit data, Falling XCK edge

// UBRR1 - USART Baud Rate Register (16-bit register, comprising UBRR0H and UBRR0L)
	UBRR1H = 0; // 9600 baud, UBRR = 12, and  U2X must be set to '1' in UCSRA
	UBRR1L = 12;
}

ISR(USART1_RX_vect) // (USART_RX_Complete_Handler) USART Receive-Complete Interrupt Handler
{
	char cData = UDR1;
	USART1_TX_SingleByte(cData); // Echo the received character
}

void USART1_TX_SingleByte(unsigned char cByte)
{
	while(!(UCSR1A & (1 << UDRE1)));	// Wait for Tx Buffer to become empty (check UDRE flag)
	UDR1 = cByte;	// Writing to the UDR transmit buffer causes the byte to be transmitted
}

void USART1_TX_String(char* str)
{
	int iStrCount;
	int iStrlen = strlen(str);
	//if(Hyperterminal_Debug_Commands_Accepted == eUSART_CommunicationMode)
	//{
		//USART1_TX_SingleByte(CR);
		//USART1_TX_SingleByte(LF);
		//USART1_TX_SingleByte('D');
		//USART1_TX_SingleByte('e');
		//USART1_TX_SingleByte('b');
		//USART1_TX_SingleByte('u');
		//USART1_TX_SingleByte('g');
		//USART1_TX_SingleByte('>');
	//}
	if(0 != iStrlen)
	{
		// need the GPIO0 wake-up on EVERY stritring write - inc when programming the ZE10 in sleepy End Device mode, because otherwise, once set
		// nt=4 the device will start sleeping - need wake-up interrupt to ensure no chars are omitted


		for(iStrCount = 0; iStrCount < iStrlen; iStrCount++)
		{
			USART1_TX_SingleByte(str[iStrCount]);
		}
		USART1_TX_SingleByte(CR);					// Always send a 'CR' - Needed for programming the ZE10 (delimits the commands)
	}
	//if(Hyperterminal_Debug_Commands_Accepted == eUSART_CommunicationMode)
	//{
		//USART_TX_SingleByte(LF);
	//}
}

void USART1_Configure_ZigBee_Coordinator()
{
	USART1_TX_String("AT&F");
	Delay_AT_F_ATZ();
	Delay_AT_F_ATZ();

	USART1_TX_String("AT+NT=1");	// NODETYPE = Sleepy End Device
	Delay_AT_ConfigurationCommand();

	USART1_TX_String("ATZ");
	Delay_AT_F_ATZ();

	USART1_TX_String("AT+UF=0");	// Flow Control (None)
	Delay_AT_ConfigurationCommand();

	//strcpy(ZigBee_AT_Command,"AT+NN=");
	//strcat(ZigBee_AT_Command,m_cDeviceName_Cached);	// Set ZigBee Node Name the same as local device name
	//USART1_TX_String(ZigBee_AT_Command);
	//Delay_AT_ConfigurationCommand();
	//
	USART1_TX_String("AT+CM=00040000"); // Set channel mask (ch 18) - needed for quick binding to Coordinator
	Delay_AT_ConfigurationCommand();

	USART1_TX_String("AT+PI=3636"); // PANID
	Delay_AT_ConfigurationCommand();

	USART1_TX_String("AT+EI=0000000000003636"); // Extended PANID
	Delay_AT_ConfigurationCommand();

//	USART1_TX_String("ATS11=1"); // Copy received data to Serial port
//	Delay_AT_ConfigurationCommand();

	USART1_TX_String("ATZ");
	Delay_AT_F_ATZ();


	USART1_TX_String("AT+PJ=255"); // Extended PANID
	Delay_AT_ConfigurationCommand();

	USART1_TX_String("ATZ");
	Delay_AT_F_ATZ();

//	USART1_TX_String("at+uc=0001950000000099,Configuration Complete");
//	Delay_AT_ConfigurationCommand();
}


//
	//int i;
	//for(i = 0; i < 5; i++)
	//{
		//USART1_TX_SingleByte(LF);
	//}
	//USART1_TX_SingleByte(CR);
	//for(i = 0; i < 20; i++)
	//{
		//USART1_TX_SingleByte('*');
	//}
	//USART1_TX_SingleByte(LF);
	//USART1_TX_SingleByte(CR);
	//for(i = 0; i < 5; i++)
	//{
		//USART1_TX_SingleByte(SPACE);
	//}
	//USART1_TX_SingleByte('U');
	//USART1_TX_SingleByte('S');
	//USART1_TX_SingleByte('A');
	//USART1_TX_SingleByte('R');
	//USART1_TX_SingleByte('T');
	//USART1_TX_SingleByte(SPACE);
	//USART1_TX_SingleByte('D');
	//USART1_TX_SingleByte('E');
	//USART1_TX_SingleByte('M');
	//USART1_TX_SingleByte('O');
	//USART1_TX_SingleByte(LF);
	//USART1_TX_SingleByte(CR);
	//for(i = 0; i < 20; i++)
	//{
		//USART1_TX_SingleByte('*');
	//}
	//USART1_TX_SingleByte(LF);
	//USART1_TX_SingleByte(CR);
//}

void Delay_AT_F_ATZ()	// AF&F and ATZ each take about 1 second, module reboots so must wait long enough
{						// 1.5 second delay is needed to ensure Factory reset or ATZ reset has completed
	Delay_1Second();
	Delay_500mS();
}

void Delay_AT_ConfigurationCommand()	// AT commands return very quickly (less than 100ms), but does return 'OK' so need to allow time for this
{
	Delay_100mS();
}

void Delay_100mS()
{
	asm volatile (
	"ldi r17,0xC0\n\t"
	"Ddelay4:\n\t"
	"ldi r16,0xFF\n\t"
	"Ddelay5:\n\t"
	"dec r16\n\t"
	"brne Ddelay5\n\t"
	"dec r17\n\t"
	"brne Ddelay4"
	:									// Output parameters (none in this case)
	:									// Input parameters in the order mapped onto %0 %1 %2
	: "%r16","%r17","%r18"				// Affected general-purpose registers (none in this case), example syntax:	"%r16","%r17","%r18"
	);
}

void Delay_500mS()
{
	Delay_100mS();
	Delay_100mS();
	Delay_100mS();
	Delay_100mS();
	Delay_100mS();
}

void Delay_1Second()
{
	asm volatile (
	"ldi r18,0x07\n\t"
	"Ddelay1:\n\t"
	"ldi r17,0xFF\n\t"
	"Ddelay2:\n\t"
	"ldi r16,0xFF\n\t"
	"Ddelay3:\n\t"
	"dec r16\n\t"
	"brne Ddelay3\n\t"
	"dec r17\n\t"
	"brne Ddelay2\n\t"
	"dec r18\n\t"
	"brne Ddelay1"
	:									// Output parameters (none in this case)
	:									// Input parameters in the order mapped onto %0 %1 %2
	: "%r16","%r17","%r18"				// Affected general-purpose registers (none in this case), example syntax:	"%r16","%r17","%r18"
	);
}