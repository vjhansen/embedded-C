
// Project 		USART - Atmel side, uses USART to interface to the Embedded Systems Workbench (which runs on a PC / laptop)
// Target 		ATMEL ATmega2560 micro-controller on STK300 board
// Program		USART_ESWB_AtmelSide_2560_LIBRARY.c
// Author		Richard Anthony
// Date			20th September 2019 LIBRARY VERSION (from original code 25th July 2019)

#pragma once

#define CR 0x0D
#define LF 0x0A
#define SPACE 0x20
#define UC unsigned char 

// Signature characters sent TO the Atmel from the PC (either from the Embedded Systems Workbench, or typed by user in Hyperterminal)
#define SIGNATURE_CHAR_1	'Q'
#define SIGNATURE_CHAR_2	'w'
#define SIGNATURE_CHAR_3	'Y'
// The Atmel will respond with each signature character incremented by 1 e.g. {'R', 'x', 'Z'}

// Connecting system sends TO the Atmel to confirm connection is complete (the final step in the handshake)
// And implicitly identifies whether it is the Embedded Systems Workbench OR a Hyperterminal user
#define SIGNATURE_CHAR_CONFIRMATION_ESWB			'E'	// Sent from Embedded Systems Workbench
#define SIGNATURE_CHAR_CONFIRMATION_Hyperterminal	'H'	// Sent from Hyperterminal (typed by user)

#define Switch_0	0b11111110
#define Switch_1	0b11111101
#define Switch_2	0b11111011
#define Switch_3	0b11110111
#define Switch_7	0b10000000

void Reset_Connection_State();
void USART0_SETUP_9600_BAUD_ASSUME_1MHz_CLOCK();
void USART0_TX_SingleByte(unsigned char cByte) ;
void USART0_TX_String(char* sData);
void USART0_DisplayBanner();
void USART0_DisplayPrompt();


volatile enum eConnectionStatus {	STATUS_DISCONNECTED, 
									STATUS_SIGNATURE_CHAR_1_RECD_RESPONSE_SENT, 
									STATUS_SIGNATURE_CHAR_2_RECD_RESPONSE_SENT,
									STATUS_SIGNATURE_CHAR_3_RECD_RESPONSE_SENT,
									STATUS_CONNECTION_ESTABLISHED_ESWB,
									STATUS_CONNECTION_ESTABLISHED_Hyperterminal };

volatile enum eConnectionStatus g_eConnectionStatus;			

volatile enum eMode_TestSequence {	MODE_0_SEND_LOOP_0_255,		// Continuously send data values to the ESWB via the serial port, auto-incrementing in range 0 - 255
									MODE_1_SEND_LOOP_A_Z,		// Continuously send data values to the ESWB via the serial port, auto-incrementing in range A - Z (as ASCII, e.g. 'A' = 65)
									MODE_2_RECV_LEDs,			// Continuously display data values on the LEDs, received from the ESWB via the serial port
									MODE_3_SEND_DATA };			// Continuously send application data values (contents of g_uData) to the ESWB via the serial port

volatile enum eMode_TestSequence g_eMode_TestSequence;						
	
UC g_iNumberCount;
UC g_iLetterCount;
UC g_iSwitchesValue;	// For Mode selection
UC g_uOperationMode;
volatile UC g_uData;