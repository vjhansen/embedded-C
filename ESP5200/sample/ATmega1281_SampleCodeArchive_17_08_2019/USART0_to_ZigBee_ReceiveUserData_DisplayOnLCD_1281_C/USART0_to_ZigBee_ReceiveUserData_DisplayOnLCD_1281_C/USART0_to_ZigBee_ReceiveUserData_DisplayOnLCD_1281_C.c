//*******************************************************************************
// Project 		USART0 - Connect to ZigBee ProBee - Receive User Data and display on LCD (Embedded C)
// Target 		ATMEL ATmega1281 micro-controller on STK300 board
// Program		USART0_to_ZigBee_ReceiveUserData_DisplayOnLCD_1281_C.c
// Author		Richard Anthony
// Date			3rd February 2014

// Note - This version specifically developed as a demo. Uses USART0 (Port E, 9-Way D'Type connector on STK300).

// Note 'User Data' is sent in the form of characters - maximum payload is 90 bytes

// Function		1. Transmits AT commands to ZigBee board to configure it
//				USART configured for serial connection to ZigBee board
//				2. Broadcasts a sign-on message as 'User Data'
//				2. Receives 'User Data' from ZigBee board (sent by a remote ZigBee board)
//				(Source of User data at remote node could be Hyperterminal connected to a ZigBee board,
//				 or an Atmel board connected to a ZigBee board
//				3.	Displays Source address on Row 0 of LCD
//					Displays Message Text on Row 1 of LCD
//				4. Broadcasts diagnostic messages when switch 0 or switch 1 are pressed (connected to port B)

// Communication setup of USART (Hyperterminal settings must match these)
//		Tx / Rx rate	Bits per second		9600
//		Data bits							   8
//		Parity								None
//		Stop bits							   1
//		Flow control						None

//	LCD position code convention
//		The LCD has two rows (0-1) and 15 columns (0-15)
//		The two values are combined into a single value POS passed to lcd_SetCursor(POS)
//		The value is derived in Hex as (Row * 0x40) + Column
//		Some examples:		Row		Column		POS value
//							0		0			0x00
//							0		1			0x01
//							0		15			0x0F
//							1		0			0x40
//							1		2			0x42
//							1		15			0x4F

// PORTs			LCD uses port A for Data and Command values (8 bits, Output)
//					The LCD 'device Busy' flag also uses bit 7 of port A
//					So sometimes this bit has to be set for Input

//					LCD uses port C for control (register select 'RS' bit 6 Output)
//												(device enable 'ENABLE' bit 7 Output)

//					LCD uses port G for control (write 'WR' bit 6 Output)
//												(read 'RD' bit 7 Output)

//					Port B (input) On-Board switches

//					USART0 RxD0 is on Port E bit 0 (Input)				9-Way D'Type connector on STK300
//					USART0 TxD0 is on Port E bit 1 (Output)

//					USART1 RxD1 is on Port D bit 2 (Input)				2-Way header pins connector on STK300
//					USART1 TxD1 is on Port D bit 3 (Output)

// FUSEs			1281 clock must be set at 1Mhz, otherwise UART clock-rate configuration must be adjusted

//*******************************************************************************

#include <avr/io.h>			// AVR studio 5.0 Default include path is C:\Program Files\Atmel\AVR Studio 5.0\AVR Toolchain\avr\include\avr
#include <avr/interrupt.h>	// May need to replace with this for AVR studio Ver4    #include <avr/signal.h>

#include <string.h>
#include <util/delay.h>
#include "LCD_LibraryFunctions_1281.h"

#define CR 0x0D
#define LF 0x0A
#define SPACE 0x20
#define QUOTE '"'
#define true 0
#define false 1
#define LCD_DISPLAY_WIDTH 16
#define ZIGBEE_COMMAND_MAXIMUM_LENGTH 40
#define ZIGBEE_USER_DATA_MAXIMUM_PAYLOAD 30 //save space    90
#define ZIGBEE_LONGADDRESS_LENGTH 16

void InitialiseGeneral();
void USART_SETUP_9600_BAUD_ASSUME_1MHz_CLOCK();
void USART_TX_SingleByte(unsigned char cByte);
void USART_TX_String(char* ZigBee_AT_Command);
void lcd_OverwriteBothRows(char* cNewStringRow0, char* cNewStringRow1);
void USART_SendCommandToZigBee_DisplayDiagnosticOnLCD();

char LCD_displayStringRow0[LCD_DISPLAY_WIDTH +1];
char LCD_displayStringRow1[LCD_DISPLAY_WIDTH +1];
char ZigBee_AT_Command[ZIGBEE_COMMAND_MAXIMUM_LENGTH +1];

// Globals for use with USART Receive Interrupt Handler
char SourceAddress[ZIGBEE_LONGADDRESS_LENGTH +1];
char ReceivedMessage[ZIGBEE_USER_DATA_MAXIMUM_PAYLOAD +1];
int iReceiveCount_Address;
int iReceiveCount_UserData;
int bCurrentIsAddress; // 0 = receiving Address, 1 = receiving User Data
int bMessageReceiptStarted; // 0 = Leading '+' NOT yet seen, 1 = Leading '+' has been seen
int bInitialisationPhase; // 0 = Initializing ZigBee board, 1 = ZigBee board Initialized

int main( void )
{
	unsigned int iSwitches_Value;
	InitialiseGeneral();
	USART_SETUP_9600_BAUD_ASSUME_1MHz_CLOCK();
	iReceiveCount_Address = 0;
	iReceiveCount_UserData = 0;
	bCurrentIsAddress = 0;		// 0 = receiving Address
	bMessageReceiptStarted = 0;	// 0 = Leading '+' NOT yet seen
	bInitialisationPhase = 0;	// 0 = Initializing ZigBee board

	lcd_Clear();				// Clear the display
	lcd_StandardMode();			// Set Standard display mode
	lcd_on();					// Set the display on
	lcd_CursorOff();			// Set the cursor display off (underscore)
	lcd_CursorPositionOff();	// Set the cursor position indicator off (flashing square)

	lcd_OverwriteBothRows("Atmel-Z'Bee link", "RJA January 2014"); // Display 'sign-on' message
	_delay_ms(2000);

	// Configure ZigBee Board - this node is End Device
	strncpy(ZigBee_AT_Command,"AT&F",ZIGBEE_COMMAND_MAXIMUM_LENGTH);
	USART_SendCommandToZigBee_DisplayDiagnosticOnLCD();
	 _delay_ms(1500);

	strncpy(ZigBee_AT_Command,"AT+NODETYPE=3",ZIGBEE_COMMAND_MAXIMUM_LENGTH);
	USART_SendCommandToZigBee_DisplayDiagnosticOnLCD();
	_delay_ms(1000);

	strncpy(ZigBee_AT_Command,"AT+PANID=1234",ZIGBEE_COMMAND_MAXIMUM_LENGTH);
	USART_SendCommandToZigBee_DisplayDiagnosticOnLCD();
	_delay_ms(1000);

	strncpy(ZigBee_AT_Command,"ATS11=1",ZIGBEE_COMMAND_MAXIMUM_LENGTH); // Copy received data to Serial port
	USART_SendCommandToZigBee_DisplayDiagnosticOnLCD();
	_delay_ms(1000);

	strncpy(ZigBee_AT_Command,"ATZ",ZIGBEE_COMMAND_MAXIMUM_LENGTH);
	USART_SendCommandToZigBee_DisplayDiagnosticOnLCD();
	_delay_ms(4000); // Longer delay to allow ZigBee board to re-initialise after the ATZ reset

	strncpy(ZigBee_AT_Command,"AT+BROADCAST=New Node 4850",ZIGBEE_COMMAND_MAXIMUM_LENGTH);  // Send a test message as diagnostic
	USART_SendCommandToZigBee_DisplayDiagnosticOnLCD();
	_delay_ms(1000);
	bInitialisationPhase = 1;	// 0 = Initializing ZigBee board, 1 = ZigBee board Initialized

	while(1)
    {
		// Requires on-board switches are connected to Port B
		iSwitches_Value = ~PINB;	// Read value on switches (Switches are inverted, so when pressed will give a '0')
		if(iSwitches_Value & 0b00000001)
		{	// Switch 0 pressed
			strncpy(ZigBee_AT_Command,"AT+BROADCAST=Sw 0 pressed",ZIGBEE_COMMAND_MAXIMUM_LENGTH);  // Broadcast a test message as diagnostic
			USART_SendCommandToZigBee_DisplayDiagnosticOnLCD();									   // Useful if 'send' end is connected to a Hyperterminal
		}
		if(iSwitches_Value & 0b00000010)
		{	// Switch 1 pressed
			strncpy(ZigBee_AT_Command,"AT+BROADCAST=Sw 1 pressed",ZIGBEE_COMMAND_MAXIMUM_LENGTH);  // Broadcast a test message as diagnostic
			USART_SendCommandToZigBee_DisplayDiagnosticOnLCD();									   // Useful if 'send' end is connected to a Hyperterminal
		}
		_delay_ms(300);
	}
}

void InitialiseGeneral()
{
	DDRA = 0xFF;			// Configure PortA direction for Output
	DDRB = 0x00;			// On-Board switches (Input)
	DDRC = 0xFF;			// Configure PortC direction for Output
	DDRE = 0b11111110;		// Configure PortE direction All output except bit 0 Input USART RxD)

	DDRG = 0xFF;			// Configure PortG direction Output (FOR LCD R/W)

	asm volatile ("SEI");	// Enable interrupts at global level set Global Interrupt Enable (I) bit
							// (also provides an example of in-line inclusion of assembly code into 'C' program)
}

void USART_SendCommandToZigBee_DisplayDiagnosticOnLCD()
{
	lcd_OverwriteBothRows(ZigBee_AT_Command,"");
	lcd_SetCursor(0x40);	// Set cursor position to line 1, col 0
	USART_TX_String(ZigBee_AT_Command);
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
	UCSR0C = 0b00000111;	// Asynchronous, No Parity, 1 stop, 8-bit data, Falling XCK edge

	// UBRR0 - USART0 Baud Rate Register (16-bit register, comprising UBRR0H and UBRR0L)
	UBRR0H = 0; // 9600 baud, UBRR = 12, and  U2X must be set to '1' in UCSRA
	UBRR0L = 12;
}


ISR(USART0_RX_vect) // (USART0_RX_Complete_Handler) USART0 Receive-Complete Interrupt Handler
{
	char cData = UDR0;

	if(0 == bInitialisationPhase)	// 0 = Initializing ZigBee board, 1 = ZigBee board Initialized
	{
		if( ('0' <= cData && '9' >= cData) ||
			('A' <= cData && 'Z' >= cData) ||
			('a' <= cData && 'z' >= cData) )
		{
			lcd_WriteChar(cData);
			return;
		}
	}
	else
	{	//	First character is '+'
		//	Next 16 characters are the source address (display on row 0 of LCD)
		//  Separation character is a pipe '|'
		//	Following characters are User Data (display on row 1 of LCD)
		//	Whole sequence is terminated with a CR or CR/LF   -------------------need to check and confirm
		//	(on receiving this pattern, clear the receive buffers)
		if(0 == bMessageReceiptStarted)
		{	// Still awaiting leading '+'
			if('+' == cData)
			{	// '+' marks the start of the data signal (automatically pre-pended at sender end)
				bMessageReceiptStarted = 1;
				iReceiveCount_Address = 0;
				iReceiveCount_UserData = 0;
				bCurrentIsAddress = 0;		// 0 = receiving Address, 1 = receiving User Data
				return;
			}
		}
		else
		{	// Message start has been detected
			if(CR == cData)
			{	// End of current message
				// Display source address on row 0 of LCD, Display User Data on row 1 of LCD
				bMessageReceiptStarted = 0;
				SourceAddress[iReceiveCount_Address] = 0; // Null terminate the string
				ReceivedMessage[iReceiveCount_UserData] = 0; // Null terminate the string
				lcd_OverwriteBothRows(SourceAddress, ReceivedMessage);
				return;
			}

			if('|' == cData)
			{	// Address completed, next is User Data
				bCurrentIsAddress = 1;		// 0 = receiving Address, 1 = receiving User Data
				return;
			}

			if(0 == bCurrentIsAddress && iReceiveCount_Address <= ZIGBEE_LONGADDRESS_LENGTH)
			{	// Currently receiving Address data
				SourceAddress[iReceiveCount_Address++] = cData;
				return;
			}

			if(1 == bCurrentIsAddress && iReceiveCount_UserData <= LCD_DISPLAY_WIDTH)
			{	// Currently receiving User Data
				ReceivedMessage[iReceiveCount_UserData++] = cData;
				return;
			}
		}
	}
}

void USART_TX_SingleByte(unsigned char cByte)
{
	while(!(UCSR0A & (1 << UDRE0)));	// Wait for Tx Buffer to become empty (check UDRE flag)
	UDR0 = cByte;	// Writing to the UDR transmit buffer causes the byte to be transmitted
}

void USART_TX_String(char* ZigBee_AT_Command)
{
	int iCount;
	int iStrlen = strlen(ZigBee_AT_Command);
	if(0 != iStrlen)
	{
		for(iCount = 0; iCount < iStrlen; iCount++)
		{
			USART_TX_SingleByte(ZigBee_AT_Command[iCount]);
		}
		USART_TX_SingleByte(CR);
	}
}

void lcd_OverwriteBothRows(char* cNewStringRow0, char* cNewStringRow1)
{
	strncpy(LCD_displayStringRow0,cNewStringRow0,LCD_DISPLAY_WIDTH);
	strncpy(LCD_displayStringRow1,cNewStringRow1,LCD_DISPLAY_WIDTH);
	lcd_Clear();
	lcd_SetCursor(0x00);		// Set cursor position to line 0, col 0
	lcd_WriteString(LCD_displayStringRow0);
	lcd_SetCursor(0x40);		// Set cursor position to line 1, col 0
	lcd_WriteString(LCD_displayStringRow1);
}