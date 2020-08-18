/*******************************************************************************
Project 		TWI Slave Tx demonstration (The TWI Master sends a request for data, and the TWI Slave sends its switches value to TWI Master)
Target 			ATmega1281 on STK300
Program			TWI_SLAVE_Tx_1281_C.c
Author			Richard Anthony
Date			8th October 2013

Fuse settings	System clock: Use the internal RC oscillator at 8.0MHz and CKDIV8 fuse programmed, resulting in 1.0MHz system clock.
				Fuse settings should be: CKSEL = "0010", SUT = "10", CKDIV8 = "0"
				ATmega1281 clock must be set at 1Mhz, otherwise UART clock-rate configuration must be adjusted

Function		Receives data from a TWI master, and displays value on LEDs
				Transmits received data values to PC Hyperterminal (optional, as diagnostic output), using the microcontroller's Serial port (USART0)
				
				LEDs connected to Port B, Switches on Port C
*******************************************************************************/
// Communication setup of USART (Hyperterminal settings must match these)
//		Tx / Rx rate	Bits per second		9600
//		Data bits							   8
//		Parity								None
//		Stop bits							   1
//		Flow control						 H/W

// PORTS
//		LEDs on PORT B
//		SWITCHES on PORT C

//		TWI on PortD
//		SCL (Clock) is on Port D bit 0 (need pullup resistor Set - Master takes care of this)
//		SDA (Data) is on Port D bit 1 (need pullup resistor Set - Master takes care of this)

//		USART on PortE (optional - for diagnostic output)
//		USART0 RxD is on Port E bit 0 (Input)
//		USART0 TxD is on Port E bit 1 (Output)

#include <avr/io.h>
#include <avr/interrupt.h>
#include <string.h>

#define CR 0x0D
#define LF 0x0A

// TWI Status codes (SLAVE - R/W are with respect to MASTER)
#define TWI_S_SLA_W_ACK				0x60
#define TWI_S_ARB_Lost_SLA_W_ACK	0x68
#define TWI_S_GenCall_ACK	 		0x70
#define TWI_S_ARB_Lost_GenCall_ACK	0x78
#define TWI_S_SLA_W_DATA_ACK		0x80
#define TWI_S_SLA_W_DATA_NACK		0x88
#define TWI_S_GenCall_DATA_ACK	 	0x90
#define TWI_S_GenCall_DATA_NACK		0x98
#define TWI_S_STOP_Rep_START	 	0xA0
#define TWI_S_SLA_R_ACK		 		0xA8
#define TWI_S_ARB_Lost_SLA_R_ACK	0xB0
#define TWI_S_DATA_Tx_ACK	 		0xB8
#define TWI_S_DATA_Tx_NACK	 		0xC0
#define  TWI_S_Last_DATA_Tx_ACK		0xC8

#define TWI_SLAVE_address 0x01	// The slave's TWI address

#define TWSR_MASK_5MSB_STATUS_BITS 0xF8		// 5 MSBs are the status code
#define TWINT_FLAG_MASK 0b10000000			// TWCR bit 7 TWINT: TWI Interrupt Flag (low signifies 'TWI busy').

// Configuration and Application-specific function prototypes
void InitialiseGeneral();

// TWI related function prototypes
void TWI_SLAVE_SETUP();
void TWINT_Clear();

// USART related function prototypes
void USART_SETUP_9600_BAUD_ASSUME_1MHz_CLOCK();
void USART_TX_SingleByte(unsigned char cByte) ;
void USART_DisplayBanner();
void USART_DisplayPrompt();
void USART_DisplayString(char* cString);
void USART_DisplayString_With_NewLine_StartAndEnd(char* cString);
void USART_DisplayString_With_NewLine_AtEnd(char* cString);

void InitialiseGeneral()
{
	// Ports
	//	PortD TWI interface on 1281 (pins controlled automatically when TWI is enabled)
	//		PortD bit 0  SCL
	//		PortD bit 1  SDA
	// TWI works with a Wire-AND bus thus needs pullup resistors (so bus is at '1' unless pulled low)
	// This code sets pullup resistors on port D TWI bits 0 and 1 (so external resistors are not needed)
	// Master takes care of this

	DDRB = 0xFF;	// PortB	on-board LEDs (Output)
	PORTB = 0xFF;	// Set all LEDs initially off (inverted on the board, so '1' = off)
	
	DDRC = 0x00;	// PortC	on-board Switches (Input)

	sei();	// Enable interrupts at global level set Global Interrupt Enable (I) bit
}


int main( void )
{
	InitialiseGeneral();
	USART_SETUP_9600_BAUD_ASSUME_1MHz_CLOCK();
	
	TWI_SLAVE_SETUP();

	USART_DisplayString_With_NewLine_StartAndEnd("******************");
	USART_DisplayString_With_NewLine_StartAndEnd("TWI Slave Tx Demo");
	USART_DisplayString_With_NewLine_StartAndEnd("******************");

	while(1)
    {
 	}
}


//**************************************************************
// TWI functionality		START
//**************************************************************
void TWI_SLAVE_SETUP()
{
	// Set TWI Control Register (TWCR)
	// bit 7 TWINT: TWI Interrupt Flag (low signifies 'TWI busy').
	//		Set by HW when TWI has finished current work and expects SW response.
	//		Reset (to '1') in software, should be done at the end of the interrupt handler
	// bit 6 TWEA: TWI Enable Ack Bit. (1 = enable the device to participate, 0 = virtually disconnect the device).
	// bit 5 TWSTA: TWI Start Condition Bit.
	//		Set to 0 initially.
	//		Set to 1 when the device wants to become the Bus MASTER. Generates START
	//		condition on the bus if it is free. Otherwise waits until a STOP condition is
	//		detected and then generates the START condition.
	//		Must be reset in SW after the START condition has been generated.
	// bit 4 TWSTO: TWI STOP Condition Bit.
	//		Normally set to 0.
	//		MASTER mode: set to 1 generates a STOP condition
	//		SLAVE mode: set to 1 resets TWI bus to a well-defined state - used in error recovery
	// bit 3 TWWC: TWI Write Collision Flag (Read only)
	//		'1' indicates attempt to write to TWI Data Reg when TWINT is low
	//		(i.e. TWI still busy with previous operation)
	//		Cleared when a subsequent to write to TWI Data Reg occurs when TWINT is high.
	// bit 2 TWEN: TWI Enable Bit (Enables TWI)
	//		Set to 1 TWI enabled, Port I/O alternate functions SCL and SDA enabled
	//		Set to 0 TWI disabled, Port I/O alternate functions SCL and SDA disabled
	// bit 1 reserved
	// bit 0 TWIE: TWI Interrupt Enable
	//		When set to 1, The TWI interrupt request will be activated while TWINT is high
	//		*** MAY NEED TO DISABLE THE INT, IF ITS CONTINUOUSLY GENERATED ***
	TWCR = (1<<TWEA)+(1<<TWEN)+(1<<TWIE)+(1<<TWINT);

	// TWI Data Register (TWDR)
	//		No need to initialise this until ready to send a specific data value

	// TWI (Slave) Address Register (TWAR)
	// bit 7,6,5,4,3,2,1 TWA (7-bit address)
	// Bit 0 TWGCE: TWI General Call Recognition Enable Bit (Set to enable recognition of a General Call)
	TWAR = (TWI_SLAVE_address * 2)/*shift left into upper 7 bits*/ +1/*Set TWGCE*/; // Slave address = ssss sss + bit 0 = 1(TWGCE)
}

//******* Interrupt Handler for TWI ******
ISR(TWI_vect)	// TWI Interrupt Handler (Wake up Slave)
{
	unsigned char TWI_SR; // Store TWI Status register value
	unsigned char uSwitchesData;
	unsigned char uInvertedSwitchesValue;
	char cReceivedCommandString[100];
	
	// Slave does nothing TWI-related before receiving an interrupt
	// i.e. TWI is set up, but the slave is not active with TWI until an interrupt occurs, 
	// signaling that the slave has been addressed 

	TWI_SR = (TWSR & TWSR_MASK_5MSB_STATUS_BITS);  // Get status code from TWSR
	
	switch(TWI_SR)
	{
		case TWI_S_SLA_R_ACK:			// 0xA8 (Slave unique address recognised, with Read flag - signifies Master is requesting data)
		case TWI_S_ARB_Lost_SLA_R_ACK:	// 0xB0
			uSwitchesData = PINC;
			TWDR = uSwitchesData;
			PORTB = uSwitchesData;
			sprintf(cReceivedCommandString,"Slave address recognised, with R,           sending %3d", uSwitchesData);
			USART_DisplayString_With_NewLine_AtEnd(cReceivedCommandString);
			break;

		case TWI_S_DATA_Tx_ACK:			// 0xB8
			uInvertedSwitchesValue = ~PINC;
			TWDR = uInvertedSwitchesValue;
			sprintf(cReceivedCommandString,"Data sent, ACK recd (Master expects more)   sending %3d", uInvertedSwitchesValue);
			USART_DisplayString_With_NewLine_AtEnd(cReceivedCommandString);
			break;
		
		case TWI_S_DATA_Tx_NACK:		// 0xC0
			USART_DisplayString_With_NewLine_AtEnd("Data sent, NACK recd (Master not expecting more)");
			break;
			
		case TWI_S_Last_DATA_Tx_ACK:	// 0xC8
			USART_DisplayString_With_NewLine_AtEnd("LAST Data sent, ACK recd (Master expects more) BUT no more to send");
			break;
	}
	TWINT_Clear();
}

void TWINT_Clear()
{	// Must 'clear' the TWINT flag in TWCR (by writing '1' to it !) It is NOT cleared automatically
	// All accesses to the registers: TWAR, TWSR and TWDR must be complete before clearing this flag
	TWCR = (1<<TWEA)+(1<<TWEN)+(1<<TWIE)+(1<<TWINT);
}
//**************************************************************
// TWI functionality		END
//**************************************************************


//**************************************************************
// USART (serial interface) functionality		START
//**************************************************************
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
	USART_TX_SingleByte(cData); // Echo the received character
}

void USART_TX_SingleByte(unsigned char cByte)
{
	while(!(UCSR0A & (1 << UDRE0)));	// Wait for Tx Buffer to become empty (check UDRE flag)
	UDR0 = cByte;	// Writing to the UDR transmit buffer causes the byte to be transmitted
}

void USART_DisplayString(char* cString)
{
	int iLen = strlen(cString);
	
	for(int i = 0; i < iLen; i++)
	{
		USART_TX_SingleByte(cString[i]);
	}
}

void USART_DisplayString_With_NewLine_StartAndEnd(char* cString)
{
	int iLen = strlen(cString);
	
	USART_TX_SingleByte(LF);
	USART_TX_SingleByte(CR);
	for(int i = 0; i < iLen; i++)
	{
		USART_TX_SingleByte(cString[i]);
	}
	USART_TX_SingleByte(LF);
	USART_TX_SingleByte(CR);
}

void USART_DisplayString_With_NewLine_AtEnd(char* cString)
{
	int iLen = strlen(cString);
	
	for(int i = 0; i < iLen; i++)
	{
		USART_TX_SingleByte(cString[i]);
	}
	USART_TX_SingleByte(LF);
	USART_TX_SingleByte(CR);
}
//**************************************************************
// USART (serial interface) functionality		END
//**************************************************************