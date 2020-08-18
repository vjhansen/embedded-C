//*******************************************************************************
// Project 		USART0 - Connect to ZigBee ProBee - Read Potentiometer values and transmit as data over ZigBee link
// Target 		ATMEL ATmega1281 micro-controller on STK300 board
// Program		USART0_to_ZigBee_SendUserDataToRemoteAtmel_1281_C.c
// Author		Richard Anthony
// Date			3rd February 2014

// Note	- This version specifically designed as Demo-Code. Uses USART0 (Port E, 9-Way D'Type connector on STK300)

// *** This program forms the user-interface side of a generic user-data application
// *** Specifically this code reads the Atmel Push buttons and a pair of potentiometers and sends user-data messages accordingly

// *** This application can also used with a Hyperterminal at receiving end to display the data values transmitted

// Note 'User Data' is sent in the form of characters - maximum payload is 90 bytes

// Function		1. Transmits AT commands to ZigBee board to configure it
//				USART configured for serial connection to ZigBee board
//				2. Broadcasts a sign-on message as 'User Data'
//				3. Reads a pair of potentiometer values via A-to-D converter - to control Camera PAN and ZOOM
//				4. Reads a pair of on-board buttons to control camera power ON / OFF
//				5. Sends 'User Data' to ZigBee board (to be forwarded to a remote ZigBee board)
//				(Sink of User data at remote node could be Hyperterminal connected to a ZigBee board instead of the PAN / TILT camera mount

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

//					USART0 RxD0 is on Port E bit 0 (Input)				9-Way D'Type connector on STK300
//					USART0 TxD0 is on Port E bit 1 (Output)

//					USART1 RxD1 is on Port D bit 2 (Input)				2-Way header pins connector on STK300
//					USART1 TxD1 is on Port D bit 3 (Output)

// FUSEs			1281 clock must be set at 1Mhz, otherwise UART clock-rate configuration must be adjusted
//*******************************************************************************
#include <avr/io.h>			// AVR studio 5.0 Default include path is C:\Program Files\Atmel\AVR Studio 5.0\AVR Toolchain\avr\include\avr
#include <avr/interrupt.h>	// May need to replace with this for AVR studio Ver4    #include <avr/signal.h>

#include <util/delay.h>
#include "LCD_LibraryFunctions_1281.h"

#define CR 0x0D
#define true 0
#define false 1
#define LCD_DISPLAY_WIDTH 16
#define ZIGBEE_COMMAND_MAXIMUM_LENGTH 50
#define ZIGBEE_USER_DATA_MAXIMUM_PAYLOAD 30 //save space    90
#define ZIGBEE_LONGADDRESS_LENGTH 16
#define Unicast_DestLA	"AT+UNICAST=000195000000484F,"

void InitialiseGeneral(void);
void Initialise_ADC(void);
void InitialiseTimer0_For_16Hz_Clock();
void USART_SETUP_9600_BAUD_ASSUME_1MHz_CLOCK();
void Start_ADC_Conversion(void);
void ADC2_HANDLER();
void ADC3_HANDLER();
void USART_TX_SingleByte(unsigned char cByte);	
void USART_SendCommandToZigBee_DisplayDiagnosticOnLCD();
void USART_TX_String();
void lcd_OverwriteBothRows(char* cNewStringRow0, char* cNewStringRow1);

char LCD_displayStringRow0[LCD_DISPLAY_WIDTH +1];
char LCD_displayStringRow1[LCD_DISPLAY_WIDTH +1];
char ZigBee_AT_Command[ZIGBEE_COMMAND_MAXIMUM_LENGTH +1];

unsigned char PotentiometerReadingA_Current;
unsigned char PotentiometerReadingA_Previous;
unsigned char PotentiometerReadingB_Current;
unsigned char PotentiometerReadingB_Previous;

int iSwitchesValue;

int main( void )
{
	InitialiseGeneral();

	lcd_Clear();				// Clear the display
	lcd_StandardMode();			// Set Standard display mode
	lcd_on();					// Set the display on
	lcd_CursorOff();			// Set the cursor display off (underscore)
	lcd_CursorPositionOff();	// Set the cursor position indicator off (flashing square)

	lcd_OverwriteBothRows("Atmel-Z'Bee link", "RJA January 2014"); // Display 'sign-on' message
	_delay_ms(2000);

	USART_SETUP_9600_BAUD_ASSUME_1MHz_CLOCK();

	// Configure ZigBee Board - this node is Coordinator (but was set as End Device in one of the video demos)
	strncpy(ZigBee_AT_Command,"AT&F",ZIGBEE_COMMAND_MAXIMUM_LENGTH);
	USART_SendCommandToZigBee_DisplayDiagnosticOnLCD();
	_delay_ms(4000); // Longer delay to allow ZigBee board to re-initialise after the AT&F reset

	strncpy(ZigBee_AT_Command,"AT+NODETYPE=1",ZIGBEE_COMMAND_MAXIMUM_LENGTH);   // Currently set as COORDINATOR
																				// Camera Actuator will be End-Device
	USART_SendCommandToZigBee_DisplayDiagnosticOnLCD();
	_delay_ms(1500);
	
	strncpy(ZigBee_AT_Command,"AT+PANID=1234",ZIGBEE_COMMAND_MAXIMUM_LENGTH);
	USART_SendCommandToZigBee_DisplayDiagnosticOnLCD();
	_delay_ms(1500);
	
	strncpy(ZigBee_AT_Command,"ATZ",ZIGBEE_COMMAND_MAXIMUM_LENGTH);
	USART_SendCommandToZigBee_DisplayDiagnosticOnLCD();
	_delay_ms(4000); // Longer delay to allow ZigBee board to re-initialise after the ATZ reset

	strncpy(ZigBee_AT_Command,"AT+PJ=255",ZIGBEE_COMMAND_MAXIMUM_LENGTH);
	USART_SendCommandToZigBee_DisplayDiagnosticOnLCD();
	_delay_ms(1500);
	
	strncpy(ZigBee_AT_Command,"ATZ",ZIGBEE_COMMAND_MAXIMUM_LENGTH);
	USART_SendCommandToZigBee_DisplayDiagnosticOnLCD();
	_delay_ms(4000); // Longer delay to allow ZigBee board to re-initialise after the ATZ reset

	Initialise_ADC();
	InitialiseTimer0_For_16Hz_Clock();

	while(1)
    {
		strcpy(ZigBee_AT_Command,Unicast_DestLA);
		// Read On-Board switches on PORT D
		iSwitchesValue = ~PIND; // On board switches are inverted

		if(0b00000001 & iSwitchesValue)
		{
			strcat(ZigBee_AT_Command,"Switch 0");
			lcd_OverwriteBothRows("Switch 0","");
			lcd_SetCursor(0x40);	// Set cursor position to line 1, col 0
			USART_TX_String(ZigBee_AT_Command);
			_delay_ms(200);
		}
		else if(0b00000010 & iSwitchesValue)
		{
			strcat(ZigBee_AT_Command,"Switch 1");
			lcd_OverwriteBothRows("Switch 1","");
			lcd_SetCursor(0x40);	// Set cursor position to line 1, col 0
			USART_TX_String(ZigBee_AT_Command);
			_delay_ms(200);
		}
		else if(0b00000100 & iSwitchesValue)
		{
			strcat(ZigBee_AT_Command,"Switch 2");
			lcd_OverwriteBothRows("Switch 2","");
			lcd_SetCursor(0x40);	// Set cursor position to line 1, col 0
			USART_TX_String(ZigBee_AT_Command);
			_delay_ms(200);
		}
		else if(0b00001000 & iSwitchesValue)
		{
			strcat(ZigBee_AT_Command,"Switch 3");
			lcd_OverwriteBothRows("Switch 3","");
			lcd_SetCursor(0x40);	// Set cursor position to line 1, col 0
			USART_TX_String(ZigBee_AT_Command);
			_delay_ms(200);
		}
		else if(0b00010000 & iSwitchesValue)
		{
			strcat(ZigBee_AT_Command,"Switch 4");
			lcd_OverwriteBothRows("Switch 4","");
			lcd_SetCursor(0x40);	// Set cursor position to line 1, col 0
			USART_TX_String(ZigBee_AT_Command);
			_delay_ms(200);
		}
		else if(0b00100000 & iSwitchesValue)
		{
			strcat(ZigBee_AT_Command,"Switch 5");
			lcd_OverwriteBothRows("Switch 5","");
			lcd_SetCursor(0x40);	// Set cursor position to line 1, col 0
			USART_TX_String(ZigBee_AT_Command);
			_delay_ms(200);
		}
 		else if(0b01000000 & iSwitchesValue)
		{
			strcat(ZigBee_AT_Command,"Switch 6");
			lcd_OverwriteBothRows("Switch 6","");
			lcd_SetCursor(0x40);	// Set cursor position to line 1, col 0
			USART_TX_String(ZigBee_AT_Command);
			_delay_ms(200);
		}
		else if(0b10000000 & iSwitchesValue)
		{
			strcat(ZigBee_AT_Command,"Switch 7");
			lcd_OverwriteBothRows("Switch 7","");
			lcd_SetCursor(0x40);	// Set cursor position to line 1, col 0
			USART_TX_String(ZigBee_AT_Command);
			_delay_ms(200);
		}
	}
}

void InitialiseGeneral()
{
	DDRA = 0xFF;			// Configure PortA direction for Output
	DDRC = 0xFF;			// Configure PortC direction for Output
	DDRD = 0x00;			// On-Board switches (Input)
	DDRE = 0b11111110;		// Configure PortE direction All output except bit 0 Input USART RxD)

	//DDRF = 0b11110011;		// Set all unused bits on port F to Output (to give an earth shield effect to the used analogue bits 2 and 3)
	//PORTF = 0x00;			// Clear all PORT F pins to provide Earth screen to the two used inputs
//
	DDRG = 0xFF;			// Configure PortG direction Output (FOR LCD R/W)

	asm volatile ("SEI");	// Enable interrupts at global level set Global Interrupt Enable (I) bit
}

void Initialise_ADC()
{
	// ADMUX – ADC Multiplexer Selection Register
	// bit7,6 Reference voltage selection (00 AREF,01 AVCC, 10 = Internal 1.1V, 11 = Internal 2.56V)
	// bit 5 ADC Left adjust the 10-bit result
	// 0 = ADCH (high) contains bit 1 = output bit 9, bit 0 = output bit 8. ADCL (low) contains output bits 7 through 0
	// 1 = ADCH (high) contains bits 9 through 2. ADCL (low) contains bit 7 = output bit 1, bit 6 = output bit 0
	// Bits 4:0 – MUX4:0: Analog Channel and Gain Selection Bits (see 1281 manual p290)
	// 00000 = ADC0 (ADC channel 0, single-ended input)
	// 00010 = ADC2 (ADC channel 2, single-ended input)
	ADMUX = 0b01100010;	// AVCC REF, Left-adjust output (Read most-significant 8 bits via ADCH), Convert channel 2

	// ADCSRA – ADC Control and Status Register A
	// bit 7 ADEN (ADC ENable) = 1 (Enabled)
	// bit 6 ADSC (ADC Start Conversion) = 0 (OFF initially)
	// bit 5 ADATE (ADC Auto Trigger Enable) = 1 (ON)
	// bit 4 ADIF (ADC Interrupt Flag) = 0 (not cleared)
	// bit 3 ADIE (ADC Interrupt Enable) = 1 (Enable the ADC Conversion Complete Interrupt)
	// bit 2,1,0 ADC clock prescaler
	// 000 = division factor 2
	// 001 = division factor 2
	// 010 = division factor 4
	// 011 = division factor 8
	// 100 = division factor 16
	// 101 = division factor 32
	// 110 = division factor 64
	// 111 = division factor 128
	ADCSRA = 0b10101101;	// ADC enabled, Auto trigger, Interrupt enabled, Prescaler = 32

	// ADCSRB – ADC Control and Status Register B
	// Bit 3 – MUX5: Analog Channel and Gain Selection Bit (always 0 when using ADC0 - ADC7)
	// Bit 2:0 – ADTS2:0: ADC Auto Trigger Source (active when ADATE bit in ADCSRA is set)
	// 0 0 0 Free Running mode
	// 0 0 1 Analog Comparator
	// 0 1 0 External Interrupt Request 0
	// 0 1 1 Timer/Counter0 Compare Match A
	// 1 0 0 Timer/Counter0 Overflow
	// 1 0 1 Timer/Counter1 Compare Match B
	// 1 1 0 Timer/Counter1 Overflow
	// 1 1 1 Timer/Counter1 Capture Event
	ADCSRB &= 0b11110100;	// Auto trigger ADC from Timer/Counter0 Overflow

	// DIDR0 – Digital Input Disable Register 0
	// Bit 7:0 – ADC7D:ADC0D: ADC7:0 Digital Input Disable
	DIDR0 = 0b00001100;	// Disable digital input on bits 2 and 3

	// DIDR2 – Digital Input Disable Register 2
	// Bit 7:0 – ADC15D:ADC8D: ADC15:8 Digital Input Disable
	DIDR2 = 0b11111111;	// Disable digital input on all bits (64-pin version of ATmega1281 does not even have these inputs)

	// Start the ADC Conversion (start first sample, runs in 'free run' mode after)
	//bit 6 ADCSRA (ADC Start Conversion) = 1 (START)
	// Read ADSCSR and OR with this value to set the flag without changing others
	ADCSRA |= 0b01000000;	// start ADC conversion
}

ISR(ADC_vect)	// ADC Interrupt Handler
{	// This interrupt handler is common for all ADC channels
	//	- Need to alternate which channel is converted
	//	- Need to set a flag so the result can be interpreted in correct context
	if(ADMUX & 0b00000001) // Check bit 0, Serves as a most-recent conversion flag (AD2 or AD3)
	{
		// NOTE: bit 0 low indicates AD2 and bit 0 high indicates AD3
		// BUT the ADC starts its next conversion immediately after the previous one ends,
		// i.e. BEFORE the new value is written to ADMUX telling it to change over.
		// Therefore when the flag is set to a particular value (assuming alternation, as in this code)
		// it means that the current conversion just finished will have been for the non-selected channel
		// and the selected channel conversion is underway at this moment.
		ADC2_HANDLER(); // Channel 3 currently selected; most recent conversion is for channel 2 - handle it.
	}
	else
	{
		ADC3_HANDLER(); // Channel 2 currently selected; most recent conversion is for channel 3 - handle it.
	}
}

void ADC2_HANDLER()
{
	ADMUX = 0b00100010;	// Set ADMUX ADC register - next conversion is for ADC2 (see note below)
	// NOTE: As soon as the most-recent conversion ended, another will have started using
	// the previous value written in ADMUX
	// So the most-recent conversion was for ADC2 (thats why we are in ADC2 handler at present)
	// During the ADC2 conversion, the ADMUX flag was set to ADC3 - this conversion is underway NOW
	// So here we must set the flag for ADC2 once again
	// (we are working with an effective lag of one conversion time)

	PotentiometerReadingA_Current = ADCH;
}

void ADC3_HANDLER()
{
	ADMUX = 0b00100011;	// Set ADMUX ADC register - next conversion is for ADC3 (see note below)
	// NOTE: As soon as the most-recent conversion ended, another will have started using
	// the previous value written in ADMUX
	// So the most-recent conversion was for ADC3 (thats why we are in ADC3 handler at present)
	// During the ADC3 conversion, the ADMUX flag was set to ADC2 - this conversion is underway NOW
	// So here we must set the flag for ADC3 once again
	// (we are working with an effective lag of one conversion time)

	PotentiometerReadingB_Current = ADCH;
}

void InitialiseTimer0_For_16Hz_Clock()		// Configure to generate an interrupt after a 1/16 Second interval
{
	TCCR0A = 0b00000000;	// Normal port operation (OC0A, OC0B), Normal waveform generation
	TCCR0B = 0b00000100;	// Normal waveform generation, Use 256 prescaler
	// For 1 MHz clock (with 256 prescaler)
	// Overflow occurs after counting to 256 (but already divided by 256)
	// So overflow occurs after (256 * 256) / 1000000 = 0.065 seconds (approx 16HZ)
	TCNT0 = 0b00000000;	// Timer/Counter count/value register

	TIMSK0 = 0b00000001;		// Use 'Overflow' Interrupt, i.e. generate an interrupt
	// when the timer reaches its maximum count value
}

ISR(TIMER0_OVF_vect) // TIMER0_Overflow_Handler (Interrupt Handler for Timer 0)
{	// State-change based transmission of changed Potentiometer values
	int iLen;
	char char1, char2, char3;
	strcpy(ZigBee_AT_Command,Unicast_DestLA);
	strncat(ZigBee_AT_Command," >   ",ZIGBEE_COMMAND_MAXIMUM_LENGTH); // The one and three spaces are back-filled (see below)

	if( PotentiometerReadingA_Current >= PotentiometerReadingA_Previous +3 || // Deadzone for stability
		PotentiometerReadingA_Current <= PotentiometerReadingA_Previous -3)
	{
		// ConvertByteToThreeCharacterSequence(PotentiometerReadingA_Current, char1, char2, char3);
		char1 = '0' + PotentiometerReadingA_Current / 100;
		char2 = '0' + (PotentiometerReadingA_Current % 100) / 10;
		char3 = '0' + PotentiometerReadingA_Current % 10;

		// The AT command-based transmission assumes character data and not binary; sends each digit that
		// makes up a character separately -i.e. the value 'CR' = 0D hex is sent as '\' '0' 'D'
		// Inserting '\\' does not completely fix the problem as each composite digit is sent, but without
		// guarantee of leading 0 insertion - making decoding harder.
		// For binary data applications the safe way to transmit 8-bit values is to decompose the
		// value programmatically into composite digits that are in the ASCII character range '0' to '9'
		// so that they are not further automatically manipulated.
		// So an 8-bit character will be sent as '0' '0' '0' to '2' '5' '5'

		iLen = strlen(ZigBee_AT_Command);
		ZigBee_AT_Command[iLen-5] = 'A';
		ZigBee_AT_Command[iLen-3] = char1;
		ZigBee_AT_Command[iLen-2] = char2;
		ZigBee_AT_Command[iLen-1] = char3;
		USART_TX_String(ZigBee_AT_Command);
		lcd_OverwriteBothRows("Potentiometer","A");
		PotentiometerReadingA_Previous = PotentiometerReadingA_Current; // Here (not ADC handler) to ensure a value sent exactly once
	}

	if( PotentiometerReadingB_Current >= PotentiometerReadingB_Previous +3 || // Deadzone for stability
		PotentiometerReadingB_Current <= PotentiometerReadingB_Previous -3)
	{
		// ConvertByteToThreeCharacterSequence(PotentiometerReadingA_Current, char1, char2, char3);
		char1 = '0' + PotentiometerReadingB_Current / 100;
		char2 = '0' + (PotentiometerReadingB_Current % 100) / 10;
		char3 = '0' + PotentiometerReadingB_Current % 10;

		// The AT command-based transmission assumes character data and not binary; sends each digit that
		// makes up a character separately -i.e. the value 'CR' = 0D hex is sent as '\' '0' 'D'
		// Inserting '\\' does not completely fix the problem as each composite digit is sent, but without
		// guarantee of leading 0 insertion - making decoding harder.
		// For binary data applications the safe way to transmit 8-bit values is to decompose the
		// value programmatically into composite digits that are in the ASCII character range '0' to '9'
		// so that they are not further automatically manipulated.
		// So an 8-bit character will be sent as '0' '0' '0' to '2' '5' '5'

		iLen = strlen(ZigBee_AT_Command);
		ZigBee_AT_Command[iLen-5] = 'B';
		ZigBee_AT_Command[iLen-3] = char1;
		ZigBee_AT_Command[iLen-2] = char2;
		ZigBee_AT_Command[iLen-1] = char3;
		USART_TX_String(ZigBee_AT_Command);
		lcd_OverwriteBothRows("Potentiometer","B");
		PotentiometerReadingB_Previous = PotentiometerReadingB_Current; // Here (not ADC handler) to ensure a value sent exactly once
	}
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
}

void USART_TX_SingleByte(unsigned char cByte)
{
	while(!(UCSR0A & (1 << UDRE0)));	// Wait for Tx Buffer to become empty (check UDRE flag)
	UDR0 = cByte;	// Writing to the UDR transmit buffer causes the byte to be transmitted
}

void USART_TX_String()
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