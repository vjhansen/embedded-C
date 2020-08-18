//*******************************************************************************
// Project 		Text-To-Speech_EMIC2 Module via serial 9600 USART1
// Target 		ATMEL ATmega1281 micro-controller on STK300 board
// Program		TextToSpeech_EMIC2Module_USART1_1281_C.c
// Author		Richard Anthony
// Date			3rd November 2017

// Function		Text-to-Speech demonstration which operates over a serial link (via the Atmel's USART1 which is provided on Port D)
//				Emic2 documentation available at: 
//					https://www.parallax.com/sites/default/files/downloads/30016-Emic-2-Text-To-Speech-Documentation-v1.2.pdf
//				Emic 2 Command Set
//				Sx Convert text-to-speech: x = message (1023 characters maximum)
//				Dx Play demonstration message: x = 0 (Speaking), 1 (Singing), 2 (Spanish)
//				X Stop playback (while message is playing)
//				Z Pause/un-pause playback (while message is playing)
//				Nx Select voice: x = 0 to 8
//				Vx Set audio volume (dB): x = -48 to 18
//				Wx Set speaking rate (words/minute): x = 75 to 600
//				Lx Select language: x = 0 (US English), 1 (Castilian Spanish), 2 (Latin Spanish)
//				Px Select parser: x = 0 (DECtalk), 1 (Epson)
//				R Revert to default text-to-speech settings
//				C Print current text-to-speech settings
//				I Print version information
//				H Print list of available commands

// Communication setup of USART1
//		Tx / Rx rate	Bits per second		9600
//		Data bits							   8
//		Parity								None
//		Stop bits							   1
//		Flow control						None

// PORTS
//			PORTB - LEDs 
//			PORTC - Switches
//			PORTD - Serial interface
//				USART1 RxD is on Port D bit 2 (Input from the EMIC2 module)
//				USART1 TxD is on Port D bit 3 (Output to the EMIC2 module)

// FUSEs		ATmega1281 clock must be set at 1Mhz, otherwise USART clock-rate configuration must be adjusted
//*******************************************************************************
#include <avr/io.h>
#include <avr/interrupt.h>
#include <string.h>
#include <stdbool.h>

#define Switch_0_Pressed 0b00000001
#define Switch_1_Pressed 0b00000010
#define Switch_2_Pressed 0b00000100
#define Switch_3_Pressed 0b00001000
#define Switch_4_Pressed 0b00010000
#define Switch_5_Pressed 0b00100000
#define Switch_6_Pressed 0b01000000
#define Switch_7_Pressed 0b10000000

#define CR 0x0D

void InitialiseGeneral();
void USART1_SETUP_9600_BAUD_ASSUME_1MHz_CLOCK();
void USART1_TX_SingleByte(unsigned char cByte);
void USART1_TX_String(char* sData);
void SpeechPhrase_1();
void SpeechPhrase_2();
void SpeechPhrase_3();
void SpeechPhrase_4();
void SpeechPhrase_5();
void SpeechPhrase_6();
void SpeechPhrase_7();
void SpeechPhrase_8();

volatile bool SpeechModuleReady;

int main( void )
{
	unsigned char ucSwitchesValue = 0;
	SpeechModuleReady = false;

	InitialiseGeneral();
	USART1_SETUP_9600_BAUD_ASSUME_1MHz_CLOCK();
	USART1_TX_SingleByte(CR);	// Cause the speech module to send the first ':' prompt	

	// Set configuration - see manual
	USART1_TX_String("V18");	// Volume -48 to 18
	USART1_TX_String("N0");		// Voice 0 to 8
	USART1_TX_String("W150");	// Speaking rate 75 to 600 words / minute
	USART1_TX_String("P0");		// Parser 0 = DECtalk, 1 = Epson

	// Speak one or more phrases
	USART1_TX_String("SPress any button");	// Note the 'S' speak command at the beginning of the text
	USART1_TX_String("W75");
	USART1_TX_String("SDo it now");
	
	while(1)
    {
 		ucSwitchesValue = ~PINC;	// Read value on switches
		PORTB = ~ucSwitchesValue;	// Diagnostic output

		// Select the phrase and speech settings using the push switches
		switch(ucSwitchesValue)
		{
			case Switch_0_Pressed:	// Switch 0 pressed
				SpeechPhrase_1();
				break;
			case Switch_1_Pressed:	// Switch 1 pressed
				SpeechPhrase_2();
				break;
			case Switch_2_Pressed:	// Switch 2 pressed
				SpeechPhrase_3();
				break;
			case Switch_3_Pressed:	// Switch 3 pressed
				SpeechPhrase_4();
				break;
			case Switch_4_Pressed:	// Switch 4 pressed
				SpeechPhrase_5();
				break;
			case Switch_5_Pressed:	// Switch 5 pressed
				SpeechPhrase_6();
				break;
			case Switch_6_Pressed:	// Switch 6 pressed
				SpeechPhrase_7();
				break;
			case Switch_7_Pressed:	// Switch 7 pressed
				SpeechPhrase_8();
				break;
		}
	}
}

void InitialiseGeneral()
{
	DDRB = 0xFF;			// Configure Port B direction Output (LEDs)
	PORTB = 0xFF;			// LEDs initially off
	
	DDRC = 0x00;			// Set Port C direction input (On-board switches to select spoken phrases)
							// On-board switches have their own external pullup resistors
	sei();					// Enable interrupts at global level set Global Interrupt Enable (I) bit
}

void USART1_SETUP_9600_BAUD_ASSUME_1MHz_CLOCK()
{
	//UCSR1A – USART Control and Status Register A
	UCSR1A = 0b00000010; // Set U2X (Double the USART Tx speed, to reduce clocking error)

	// UCSR1B - USART Control and Status Register B
	UCSR1B = 0b10011000;  // RX Complete Int Enable, RX Enable, TX Enable, 8-bit data

	// UCSR1C - USART Control and Status Register C
	// *** This register shares the same I/O location as UBRRH ***
	UCSR1C = 0b00000111;	// Asynchronous, No Parity, 1 stop, 8-bit data, Falling XCK edge

	// UBRR1 - USART0 Baud Rate Register (16-bit register, comprising UBRR0H and UBRR0L)
	UBRR1H = 0; // 9600 baud, UBRR = 12, and  U2X must be set to '1' in UCSRA
	UBRR1L = 12;
}
		
ISR(USART1_RX_vect) // USART1 Receive-Complete Interrupt Handler
{	
	char cData = UDR1;
	
	switch(cData)
	{
		case ':' :						// The speech module has sent the ':' prompt	
			SpeechModuleReady = true;
			break;
	}
}

void USART1_TX_SingleByte(unsigned char cByte) 
{
	while(!(UCSR1A & (1 << UDRE1)));	// Wait for Tx Buffer to become empty (check UDRE flag)
	UDR1 = cByte;	// Writing to the UDR transmit buffer causes the byte to be transmitted
}

void USART1_TX_String(char* sData)
{
	int iCount;
	int iStrlen = strlen(sData);
	
	while(false == SpeechModuleReady);	// If speech module is not ready, wait for it to become ready
	// Each command must end with CR (or \n), the speech module responds with the ':' prompt when ready for next command
	SpeechModuleReady = false;
	
	if(0 != iStrlen)
	{
		for(iCount = 0; iCount < iStrlen; iCount++)
		{
			USART1_TX_SingleByte(sData[iCount]);
		}
		USART1_TX_SingleByte(CR);
	}
}

void SpeechPhrase_1()
{
	// Set configuration - see manual
	USART1_TX_String("V18");	// Volume -48 to 18
	USART1_TX_String("N0");		// Voice 0 to 8
	USART1_TX_String("W100");	// Speaking rate 75 to 600 words / minute 
	USART1_TX_String("P0");		// Parser 0 = DECtalk, 1 = Epson

	// Speak one or more phrases
	USART1_TX_String("SHello");				// Note the 'S' speak command at the beginning of the text
	USART1_TX_String("SSample Phrase one");
	USART1_TX_String("SGoodbye");
}

void SpeechPhrase_2()
{
	// Set configuration - see manual
	USART1_TX_String("V18");	// Volume -48 to 18
	USART1_TX_String("N1");		// Voice 0 to 8
	USART1_TX_String("W140");	// Speaking rate 75 to 600 words / minute
	USART1_TX_String("P0");		// Parser 0 = DECtalk, 1 = Epson

	// Speak one or more phrases
	USART1_TX_String("SHello");				// Note the 'S' speak command at the beginning of the text
	USART1_TX_String("SSample Phrase two");	
	USART1_TX_String("SGoodbye");
}

void SpeechPhrase_3()
{
	// Set configuration - see manual
	USART1_TX_String("V18");	// Volume -48 to 18
	USART1_TX_String("N2");		// Voice 0 to 8
	USART1_TX_String("W160");	// Speaking rate 75 to 600 words / minute
	USART1_TX_String("P0");		// Parser 0 = DECtalk, 1 = Epson

	// Speak one or more phrases
	USART1_TX_String("SHello");				// Note the 'S' speak command at the beginning of the text
	USART1_TX_String("SSample Phrase three");
	USART1_TX_String("SGoodbye");
}

void SpeechPhrase_4()
{
	// Set configuration - see manual
	USART1_TX_String("V18");	// Volume -48 to 18
	USART1_TX_String("N3");		// Voice 0 to 8
	USART1_TX_String("W200");	// Speaking rate 75 to 600 words / minute
	USART1_TX_String("P0");		// Parser 0 = DECtalk, 1 = Epson

	// Speak one or more phrases
	USART1_TX_String("SHello");				// Note the 'S' speak command at the beginning of the text
	USART1_TX_String("SSample Phrase four");
	USART1_TX_String("SGoodbye");
}

void SpeechPhrase_5()
{
	// Set configuration - see manual
	USART1_TX_String("V18");	// Volume -48 to 18
	USART1_TX_String("N4");		// Voice 0 to 8
	USART1_TX_String("W100");	// Speaking rate 75 to 600 words / minute
	USART1_TX_String("P1");		// Parser 0 = DECtalk, 1 = Epson

	// Speak one or more phrases
	USART1_TX_String("SHello");				// Note the 'S' speak command at the beginning of the text
	USART1_TX_String("SSample Phrase five");
	USART1_TX_String("SGoodbye");
}

void SpeechPhrase_6()
{
	// Set configuration - see manual
	USART1_TX_String("V18");	// Volume -48 to 18
	USART1_TX_String("N6");		// Voice 0 to 8
	USART1_TX_String("W140");	// Speaking rate 75 to 600 words / minute
	USART1_TX_String("P1");		// Parser 0 = DECtalk, 1 = Epson

	// Speak one or more phrases
	USART1_TX_String("SHello");				// Note the 'S' speak command at the beginning of the text
	USART1_TX_String("SSample Phrase six");
	USART1_TX_String("SGoodbye");
}

void SpeechPhrase_7()
{
	// Set configuration - see manual
	USART1_TX_String("V18");	// Volume -48 to 18
	USART1_TX_String("N7");		// Voice 0 to 8
	USART1_TX_String("W160");	// Speaking rate 75 to 600 words / minute
	USART1_TX_String("P1");		// Parser 0 = DECtalk, 1 = Epson

	// Speak one or more phrases
	USART1_TX_String("SHello");				// Note the 'S' speak command at the beginning of the text
	USART1_TX_String("SSample Phrase seven");
	USART1_TX_String("SGoodbye");
}

void SpeechPhrase_8()
{
	// Set configuration - see manual
	USART1_TX_String("V18");	// Volume -48 to 18
	USART1_TX_String("N8");		// Voice 0 to 8
	USART1_TX_String("W200");	// Speaking rate 75 to 600 words / minute
	USART1_TX_String("P1");		// Parser 0 = DECtalk, 1 = Epson

	// Speak one or more phrases
	USART1_TX_String("SHello");				// Note the 'S' speak command at the beginning of the text
	USART1_TX_String("SSample Phrase eight");
	USART1_TX_String("SGoodbye");
}