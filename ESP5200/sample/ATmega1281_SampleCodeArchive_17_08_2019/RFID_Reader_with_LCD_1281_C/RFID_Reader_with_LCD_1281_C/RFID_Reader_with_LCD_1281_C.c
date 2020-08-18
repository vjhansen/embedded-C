// Project 		RFID Reader with LCD output
// Target 		ATmega1281 on STK300
// Program		RFID_Reader_with_LCD_1281_C.asm
// Author		Richard Anthony
// Date			8th November 2013 (assembly version for ATmega8535 16th January 2010)

// NOTE - Connect RFID reader on PORT E bit 0 (RXD0)

// Main Function	Use of RFID reader ID-12
//					Configures RFID reader for ASCII output data format
//					RFID output is connected to RXD (USART input), alternate function of PORT E bit 0
//					Uses a custom include file which provides subroutines for configuring the USART
//					for connection to the RFID receiver and for receiving and validating the RFID tag data.

//	RFID data characteristics
//					Transmission rate 9600 Baud
//					There is no separate clock provided, so the USART is used in Asynchronous mode
//					The message format from the RFID receiver comprises 16 bytes configured as below:
//					Byte	Value	Meaning							Offset in RFID Library receive buffer
//					1		02H		STX (Start of Transmission)
//					2		?		Data byte # 1	(ASCII encoded)
//					3		?		Data byte # 2	(ASCII encoded)
//					4		?		Data byte # 3	(ASCII encoded)
//					5		?		Data byte # 4	(ASCII encoded)
//					6		?		Data byte # 5	(ASCII encoded)
//					7		?		Data byte # 6	(ASCII encoded)
//					8		?		Data byte # 7	(ASCII encoded)
//					9		?		Data byte # 8	(ASCII encoded)
//					10		?		Data byte # 9	(ASCII encoded)
//					11		?		Data byte # 10	(ASCII encoded)
//					12		?		Checksum (Exclusive OR of the 10 Data bytes) ?
//					13		?		Checksum (Exclusive OR of the 10 Data bytes) ?
//					14		0DH		ASCII Carriage return character
//					15		0AH		ASCII Line Feed character
//					16		03H		ETX (End of Transmission)

// Function LCD output
//					Uses a custom include file which provides subroutines for
//					configuring the LCD and displaying data onto it.

//					Two types of message are used in this application:
//					1. hard-coded messages "GOOD" and "ERROR" are displayed depending on whether the
//					RFID tag was read successfully, or erroneously, respectively.
//					2. Dynamic message - the data read from the RFID tag is placed into a buffer;
//					this represents the tag's unique ID

// LDC I/O requirements
//					Uses the custom LCD interface connection on the STK300
// 					Uses port A for Data and Command values (8 bits, Output)
//					LCD 'device Busy' flag also uses PORTA bit 7 (so this bit is sometimes set for Input)
//					Uses port C for control (register select 'RS' bit 6 Output)
//											(device enable 'ENABLE' bit 7 Output)
//					Uses port G for control (write 'WR' bit 6 Output)
//											(read 'RD' bit 7 Output)

//	Port configurations
//					Port A bits 0-7 used by the LCD Display
//					Port B bits 0-7 used for diagnostics (on-board LEDs) - not necessary for correct operation
//					Port C bits 6-7 used by the LCD Display
//					Port E bit 0 (USART RXD) is used by the RFID reader
//					Port D bit 0 (Switches connected to H/W INT0 and INT1)
//					Port G bits 0-1 used by the LCD Display
//********************************************************************

#include <avr/io.h>
#include <avr/interrupt.h>
#include <string.h>

#include "LCD_LibraryFunctions_1281.h"

#include "RFID_Reader_1281.h"
#include "USART0_Configuration_For_RFID_1281.h"

void InitialiseGeneral(void);
void Initialise_INT0();		// PORT D - Switch 0 to reset reader system and clear LCD
void Initialise_INT1();		// PORT D - Switch 1 to compare RFID TAG ID read with pre-stored reference TAG ID

int main(void)
{
	RFID_BYTE_COUNT = 0;

	Initialise_INT0();
	Initialise_INT1();
	USART_SETUP_9600_BAUD_ASSUME_1MHz_CLOCK();
	InitialiseGeneral();

	lcd_Clear();			// Clear the display
	lcd_StandardMode();		// Set Standard display mode
	lcd_on();				// Set the display on
	lcd_CursorOff();			// Set the cursor display off (underscore)
	lcd_CursorPositionOff();	// Set the cursor position indicator off (flashing square)

	lcd_SetCursor(0x05);	// Set cursor position to line 1, col 1

	while(1)
	{
		//TODO:: Please write your application code
	}
}

void InitialiseGeneral()
{
	DDRB = 0xFF;		// Configure PortB direction for Output
	PORTB = 0xFF;		// Set all LEDs initially off (inverted on the board, so '1' = off)

	// Ports for LCD
	DDRA = 0xFF;		// Set port A (Data and Command) as output
	DDRC = 0xFF;		// Set port C (bits 6 and 7 for RS and ENABLE) as output
	PORTC = 0x00;		// RS and ENABLE initially low
	DDRG = 0x3F;		// Set port G (bits 0 and 1 for RD and WR) as output

	DDRD - 0x00;		// Port D bit 0 input (Switches connected to H/W INT0)

	DDRE = 0x00;		// Port E bit 0 (read data from RFID reader set for input)
	
	sei();				// Enable interrupts at global level, set Global Interrupt Enable (I) bit
}

void Initialise_INT0()		// PORT D - Switch 0 to clear LCD
{
	// EICRA – External Interrupt Control Register A
	// Bits 7:0 – ISC31, ISC30 – ISC00, ISC00: External Interrupt 3 - 0 Sense Control Bits
	// 10  = The falling edge of INTn generates asynchronously an interrupt request
	EICRA = 0b00000010;		// Interrupt Sense (INT0) falling-edge triggered
	
	// EICRB – External Interrupt Control Register B
	// Bits 7:0 – ISC71, ISC70 - ISC41, ISC40: External Interrupt 7 - 4 Sense Control Bits
	// 10  = The falling edge of INTn generates asynchronously an interrupt request
	EICRB = 0b00000000;
	
	// EIMSK – External Interrupt Mask Register
	// Bits 7:0 – INT7:0: External Interrupt Request 7 - 0 Enable
	EIMSK |= 0b00000001;	// Set bit 0 to Enable H/W Int 0
	
	// EIFR – External Interrupt Flag Register
	// Bits 7:0 – INTF7:0: External Interrupt Flags 7 - 0
	EIFR = 0b11111111;		// Clear all HW interrupt flags (in case a spurious interrupt has occurred during chip startup)
}

void Initialise_INT1()		// PORT D - Switch 1 to compare RFID TAG ID read with pre-stored reference TAG ID
{
	// EICRA – External Interrupt Control Register A
	// Bits 7:0 – ISC31, ISC30 – ISC00, ISC00: External Interrupt 3 - 0 Sense Control Bits
	// 10  = The falling edge of INTn generates asynchronously an interrupt request
	EICRA = 0b00001000;		// Interrupt Sense (INT0) falling-edge triggered
	
	// EICRB – External Interrupt Control Register B
	// Bits 7:0 – ISC71, ISC70 - ISC41, ISC40: External Interrupt 7 - 4 Sense Control Bits
	// 10  = The falling edge of INTn generates asynchronously an interrupt request
	EICRB = 0b00000000;
	
	// EIMSK – External Interrupt Mask Register
	// Bits 7:0 – INT7:0: External Interrupt Request 7 - 0 Enable
	EIMSK |= 0b00000010;	// Set bit 1 to Enable H/W Int 1
	
	// EIFR – External Interrupt Flag Register
	// Bits 7:0 – INTF7:0: External Interrupt Flags 7 - 0
	EIFR = 0b11111111;		// Clear all HW interrupt flags (in case a spurious interrupt has occurred during chip startup)
}

//void Initialise_PCINT8()	// RFID Input signal on PORT E bit 0
//{
	////****************************************************************************************************
	//// Ideally would like to mimic the previous 8535 version, in which falling -edge trigger was used
	//// as PC ints work on any change it might be necessary to check the bit, and ignore if its a 1 (as it means a rising edge caused the interrupt)
	//// however, this code may work ok - so leave this aspect until 
	////****************************************************************************************************
	//
	//// PCICR – Pin Change Interrupt Control Register
	//// Bit 2 – PCIE2: Pin Change Interrupt Enable 2 (PCINT23:16)
	//// Bit 1 – PCIE1: Pin Change Interrupt Enable 1 (PCINT15:8)
	//// Bit 0 – PCIE0: Pin Change Interrupt Enable 0 (PCINT7:0)
	//PCICR = 0b00000010;		// PCINT 8 is needed
	//
	//// PCIFR – Pin Change Interrupt Flag Register
	//// Bit 2 – PCIF2: Pin Change Interrupt Flag 2 (PCINT23:16)
	//// Bit 1 – PCIF1: Pin Change Interrupt Flag 1 (PCINT15:8)
	//// Bit 0 – PCIF0: Pin Change Interrupt Flag 0 (PCINT7:0)
	//PCIFR = 0b00000111;		// Clear all PCINT interrupt flags (in case a spurious interrupt has occurred during chip startup)
	//
	//// PCMSK2 – Pin Change Mask Register 2
	//// Bit 7:0 – PCINT23:16: Pin Change Enable Mask 23:16
	//// PCMSK2 = 0b00000000;
	//
	//// PCMSK1 – Pin Change Mask Register 1
	//// Bit 7:0 – PCINT15:8: Pin Change Enable Mask 15:8
	//PCMSK1 = 0b00000001;	// Enable PCINT 8 
	//
	//// PCMSK0 – Pin Change Mask Register 0
	//// Bit 7:0 – PCINT7:0: Pin Change Enable Mask 7:0
	//PCMSK0 - 0b00000000;
//}

// ***** Hardware Interrupt Handlers
ISR(INT0_vect)	// Reset the reader system
{
	lcd_Clear();	// Clear the LCD display
	RFID_BYTE_COUNT = 0;
}

ISR(INT1_vect)
{
	RFID_TAG_COMPARE(); // Compare the RFID tag against the reference tag ID
}