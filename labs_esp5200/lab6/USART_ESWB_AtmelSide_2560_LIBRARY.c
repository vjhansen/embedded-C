//*******************************************************************************
// Project 		USART - Atmel side, uses USART to interface to the Embedded Systems Workbench (which runs on a PC / laptop)
// Target 		ATMEL ATmega2560 micro-controller on STK300 board
// Program		USART_ESWB_AtmelSide_2560_LIBRARY.c
// Author		Richard Anthony
// Date			20th September 2019 LIBRARY VERSION (from original code 25th July 2019)

// Function		Configures serial port for connection to the Embedded Systems Workbench (ESWB)
//				Operates a simple handshaking routine which enables the ESWB to 'discover' the Atmel automatically
//				Note 1 - The ESWB does not need to know which serial (COM) port the Atmel is connected to.
//				Note 2 - A serial-to-USB converter cable may be needed if the PC / Laptop does not have a traditional 9-pin serial (COM) port.

//				The ESWB has a number of functions to display data sent by the Atmel board (including graphs, simple logic analyser and save as log file)
//				- in order to demonstrate the use of these ESWB functions this program generates data values that sequence 0 - 255, or A - Z.

//				The ESWB has a number of functions to generate data that is sent to the Atmel board (e.g. to simulate particular sensors, and to control servos)
//				- in order to demonstrate the use of these ESWB functions this program has a mode to continuously display data values received from the ESWB

//				In addition, this program is designed to serve as a template / starting point for the development of more sophisticated Atmel-side applications
//				Which use the features of the ESWB, e.g. to display and log output on the PC for diagnostic purposes, or which require that a particular sensor 
//				is simulated precisely, such as having range-limited input values, or having a signal that contains a certain noise extent)


//				The operation modes are 
//				Switch 0	MODE_0_SEND_LOOP_0_255		// Continuously send data values to the ESWB via the serial port, auto-incrementing in range 0 - 255
//				Switch 1	MODE_1_SEND_LOOP_A_Z		// Continuously send data values to the ESWB via the serial port, auto-incrementing in range A - Z (as ASCII, e.g. 'A' = 65)
//				Switch 2	MODE_2_RECV_LEDs			// Continuously display data values on the LEDs, received from the ESWB via the serial port
//				Switch 3	MODE_3_SEND_DATA			// ADDED in LIBRARY VERSION - sends value in uData global variable
//				Switch 7	Reset_Connection_State		// Reset the serial port connection state - to support the ESWB auto-discovery of, and connection to, the Atmel
//				*** Note - switch inputs are not necessary - one suitable MODE can be hardcoded in Main code***


//				Communication setup of USART (Hyperterminal settings must match these)
//				Tx / Rx rate	Bits per second		9600
//				Data bits							   8
//				Parity								None
//				Stop bits							   1
//				Flow control						None
//				Note: The USART configuration assumes the Atmel clock is 1MHz; for other clock rates modify the baudrate settings of the USART as necessary


// Automatic COM port discovery protocol
//				The Atmel connects to a PC via a serial to USB adapter, which is mapped to a virtual COM port on the PC
//				However, the actual COM port number allocated is not known in advance.
//				A handshaking protocol has been devised to:
//				- enable the ESWB on the PC side to automatically detect the presence of the Atmel serial connection
//				- confirm to both sides that the connection is operative and that the connection parameters (esp. 9600 baud) are correct
//
//				The protocol operates as follows:
//				- The PC serial port management builds a list of all available COM ports based on device connectivity
//				- The PC USART transmits three 'signature' characters to the Atmel
//				- The Atmel receives these characters and if the values and sequence are correct:
//				  the Atmel responds with each signature character incremented by 1, e.g. {'Q', 'w', 'Y'} becomes {'R', 'x', 'Z'}
//				- The PC validates this response and if correct sends the final single confirmation character (*** see below ***)
//				  (at this point the PC side considers the connection fully established
//				- The Atmel receives the confirmation character, if correct considers the connection fully established
//				The final single confirmation character can be one of two values, to signify whether the PC side is running the ESWB or a hyperterminal:
//					SIGNATURE_CHAR_CONFIRMATION_ESWB			'E'	// Sent from Embedded Systems Workbench
//					SIGNATURE_CHAR_CONFIRMATION_Hyperterminal	'H'	// Sent from Hyperterminal (the entire PC-side protocol sequence must be typed by user)



// Atmel I/O PORTS
//		Port E bits 0-1		Serial communication with external computer
//								USART0 RxD is on Port E bit 0 (Input)
//								USART0 TxD is on Port E bit 1 (Output)

// FUSEs		ATmega2560 clock must be set at 1Mhz, otherwise UART clock-rate configuration must be adjusted
//*******************************************************************************

#define F_CPU 1000000
#include <avr/io.h>
#include <avr/interrupt.h>
#include <util/delay.h>
#include <string.h>
#include <stdbool.h>
#include "USART_ESWB_AtmelSide_2560_LIBRARY.h"

// Original code (from standalone demo version) left here for documentation purposes
// To use this as a LIBRARY, there can be no main() here
// Copy the relevant parts from the commented code below to your application's main()
//int main( void )
//{
	//InitialiseGeneral();
	//USART0_SETUP_9600_BAUD_ASSUME_1MHz_CLOCK();
	//Reset_Connection_State();
	//
	//while(1)
    //{
		//switch (g_eConnectionStatus) {
			//case STATUS_DISCONNECTED:
				//break;
			//case STATUS_SIGNATURE_CHAR_1_RECD_RESPONSE_SENT:
				//break;
			//case STATUS_SIGNATURE_CHAR_2_RECD_RESPONSE_SENT:
				//break;
			//case STATUS_SIGNATURE_CHAR_3_RECD_RESPONSE_SENT:
				//break;
			//case STATUS_CONNECTION_ESTABLISHED_ESWB:
				//switch (g_eMode_TestSequence) {
					//case MODE_0_SEND_LOOP_0_255:
						//USART0_TX_SingleByte(g_iNumberCount++);
						//if(255 < g_iNumberCount)
						//{
							//g_iNumberCount = 0;
						//}
						//PORTK = g_iNumberCount;
						//break;
					//case MODE_1_SEND_LOOP_A_Z:
						//USART0_TX_SingleByte(g_iLetterCount++);
						//if('Z' < g_iLetterCount)
						//{
							//g_iLetterCount = 'A';
						//}
						//PORTK = g_iLetterCount;
						//break;
					//case MODE_2_RECV_LEDs:
						//break;
				//}
				//break;				
			//case STATUS_CONNECTION_ESTABLISHED_Hyperterminal:
				//switch (g_eMode_TestSequence) {
					//case MODE_0_SEND_LOOP_0_255:
						//USART0_TX_SingleByte(g_iNumberCount++);
						//if(255 < g_iNumberCount)
						//{
							//g_iNumberCount = 0;
						//}
						//PORTK = g_iNumberCount;
						//break;
					//case MODE_1_SEND_LOOP_A_Z:
						//USART0_TX_SingleByte(g_iLetterCount++);
						//if('Z' < g_iLetterCount)
						//{
							//g_iLetterCount = 'A';
						//}
						//PORTK = g_iLetterCount;
						//break;
					//case MODE_2_RECV_LEDs:
						//break;
				//}
				//break;
			//default:
				//break;
			//
 		//}
//
		//// Scan Switches (port D) for Mode selection
		//// Port D is used in Active-Low configuration (pins pulled up to '1' state when not active)
		//// To activate an input the chosen pin is connected to 0V via a switch
		//// Therefore the port scan is looking for any port pins that read as a '0', this signifies the user's input choice.
		//g_iSwitchesValue = PIND;	// Read value on switches (e.g. 11111101 means switch 1 was pressed)
//
		//switch(g_iSwitchesValue)
		//{
			//case Switch_0_Pressed:
				//g_eMode_TestSequence = MODE_0_SEND_LOOP_0_255;
				//_delay_ms(80);	// Switch debounce delay
				//break;
			//case Switch_1_Pressed:
				//g_eMode_TestSequence = MODE_1_SEND_LOOP_A_Z;
				//_delay_ms(80);	// Switch debounce delay
				//break;
			//case Switch_2_Pressed:
				//g_eMode_TestSequence = MODE_2_RECV_LEDs;
				//_delay_ms(80);	// Switch debounce delay
				//break;
			//case Switch_7_Pressed:
				//Reset_Connection_State();
				//_delay_ms(80);	// Switch debounce delay
				//break;
		//}
//
		//_delay_ms(100); // Keep separate to switch debounce delay which only occurs when a switch is pressed
	//}
//}

void Reset_Connection_State()
{
	g_eConnectionStatus = STATUS_DISCONNECTED;
	g_eMode_TestSequence = MODE_0_SEND_LOOP_0_255;
	g_iNumberCount = 0;
	g_iLetterCount = 'A';
	PORTK = 0b00111100;
}

// Original code (from standalone demo version) left here for documentation purposes
// To use this as a LIBRARY, cannot duplicate  InitialiseGeneral() here
// Copy the relevant parts from the commented code below to your application's InitialiseGeneral()
//void InitialiseGeneral()
//{
	//DDRK = 0xFF;	// Configure PortK direction for Output (LEDs)
	//PORTK = 0x00;	// LEDs initially off
//
	//DDRD = 0b00000000;	// Configure PortD direction for Input (switches)
	//PORTD = 0b11111111;	// Configure PortD pullup resistors ON
//
	//sei();			// Enable interrupts at global level set Global Interrupt Enable (I) bit
//}

void USART0_SETUP_9600_BAUD_ASSUME_1MHz_CLOCK()
{
	//UCSR0A – USART Control and Status Register A
	UCSR0A = 0b00000010; // Set U2X (Double the USART Tx speed, to reduce clocking error)

	// UCSR0B - USART Control and Status Register B
	UCSR0B = 0b10011000;  // RX Complete Int Enable, RX Enable, TX Enable, 8-bit data

	// UCSR0C - USART Control and Status Register C  (*** This register shares the same I/O location as UBRRH ***)
	UCSR0C = 0b00000111;	// Asynchronous, No Parity, 1 stop, 8-bit data, Falling XCK edge

	// UBRR0 - USART0 Baud Rate Register (16-bit register, comprising UBRR0H and UBRR0L)
	UBRR0H = 0; // 9600 baud, UBRR = 12, and  U2X must be set to '1' in UCSRA
	UBRR0L = 12;
}
		
ISR(USART0_RX_vect) // (USART0_RX_Complete_Handler) USART0 Receive-Complete Interrupt Handler
{	
	//static int iSignatureReceivedByteCount = 0;
	char cData = UDR0;
		
	// Handshaking - detect signature character sequence from PC and respond by returning incremented instances of each character

		// Atmel (A) to hyperterminal (H) handshaking requires the user (on hyperterminal) to type one char at a time and then the Atmel responding,
		// in sequence until the entire sequence has been exchanged,
		// i.e.
		// H -> A	'Q'
		// A -> H		'R'
		// H -> A	'w'
		// A -> H		'x'
		// H -> A	'Y'
		// A -> H		'Z'
		
		// H -> A	'H'  (connection complete - this is just the confirmation char TO the Atmel    - if using Hyperterminal native)
		// H -> A	'E'  (connection complete - this is just the confirmation char TO the Atmel    - if using Hyperterminal to emulate ESWB)
		// This needs to be compatible with the auto-detect mechanism that is instigated in the Embedded Systems Workbench
		// Sequence typed from Hyperterminal is thus QwYH

	switch (g_eConnectionStatus) {
		case STATUS_DISCONNECTED:
			if(SIGNATURE_CHAR_1 == cData) // ESWB will generate automatically, Hyperterminal user must type char
			{
				USART0_TX_SingleByte(SIGNATURE_CHAR_1+1);
				g_eConnectionStatus = STATUS_SIGNATURE_CHAR_1_RECD_RESPONSE_SENT;
			}
			break;
		case STATUS_SIGNATURE_CHAR_1_RECD_RESPONSE_SENT:
			if(SIGNATURE_CHAR_2 == cData) // ESWB will generate automatically, Hyperterminal user must type char
			{
				USART0_TX_SingleByte(SIGNATURE_CHAR_2+1);
				g_eConnectionStatus = STATUS_SIGNATURE_CHAR_2_RECD_RESPONSE_SENT;
			}
			break;
		case STATUS_SIGNATURE_CHAR_2_RECD_RESPONSE_SENT:
			if(SIGNATURE_CHAR_3 == cData) // ESWB will generate automatically, Hyperterminal user must type char
			{
				USART0_TX_SingleByte(SIGNATURE_CHAR_3+1);
				g_eConnectionStatus = STATUS_SIGNATURE_CHAR_3_RECD_RESPONSE_SENT;
			}
			break;
		case STATUS_SIGNATURE_CHAR_3_RECD_RESPONSE_SENT:
			if(SIGNATURE_CHAR_CONFIRMATION_ESWB == cData) // ESWB will generate automatically 'E'
			{
				g_eConnectionStatus = STATUS_CONNECTION_ESTABLISHED_ESWB;
				// USART0_DisplayBanner();  // do not send banner if connected to ESWB
				PORTK = (unsigned char) 0b10001111;
				break;
			}
			if(SIGNATURE_CHAR_CONFIRMATION_Hyperterminal == cData) // Hyperterminal user must type char 'H'
			{
				g_eConnectionStatus = STATUS_CONNECTION_ESTABLISHED_Hyperterminal;
				USART0_DisplayBanner();
				PORTK = 0b00101111;
				break;
			}
			break;
		case STATUS_CONNECTION_ESTABLISHED_ESWB:
			switch (g_eMode_TestSequence) {
				case MODE_0_SEND_LOOP_0_255:
					break;
				case MODE_1_SEND_LOOP_A_Z:
					break;
				case MODE_2_RECV_LEDs:
					PORTK = cData;
					break;
				case MODE_3_SEND_DATA:
					break;
			}
			break;
		case STATUS_CONNECTION_ESTABLISHED_Hyperterminal:
			switch (g_eMode_TestSequence) {
				case MODE_0_SEND_LOOP_0_255:
					break;
				case MODE_1_SEND_LOOP_A_Z:
					break;
				case MODE_2_RECV_LEDs:
					PORTK = cData;
					break;
				case MODE_3_SEND_DATA:
					break;
			}
			break;
		default:
			break;
	}
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

void USART0_DisplayBanner()
{
	USART0_TX_String("\r\n\n**********************************************************");
	USART0_TX_String("* Atmel-side demonstrator for Embedded Systems Workbench *");
	USART0_TX_String("**********************************************************");
}
	
void USART0_DisplayPrompt()
{
	USART0_TX_SingleByte(LF);	
	USART0_TX_SingleByte('A');	
	USART0_TX_SingleByte('t');
	USART0_TX_SingleByte('m');	
	USART0_TX_SingleByte('e');
	USART0_TX_SingleByte('l');	
	USART0_TX_SingleByte('>');
}