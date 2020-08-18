/*******************************************************************************
Project 		TWI Master side for reading Humidity and Temperature sensor (TWI SLAVE)
Target 			ATmega1281 on STK300
Program			TWI_Master_For_HumiditySensorSLAVE_1281_C.c
Author			Richard Anthony
Date			6th October 2013

Fuse settings	System clock: Use the internal RC oscillator at 8.0MHz and CKDIV8 fuse programmed, resulting in 1.0MHz system clock.
				Fuse settings should be: CKSEL = "0010", SUT = "10", CKDIV8 = "0"
				ATmega1281 clock must be set at 1Mhz, otherwise UART clock-rate configuration must be adjusted

Function		Reads Humidity and Temperature values from I2C (TWI) Humidity and Temperature sensor.
				Sensor type: Hygrochip Digital Humidity Sensor
				Displays Humidity value on on-board LEDs
				Transmits Humidity and Temperature values to PC Hyperterminal, using the microcontroller's Serial port (USART0)
*******************************************************************************/
// Communication setup of USART (Hyperterminal settings must match these)
//		Tx / Rx rate	Bits per second		9600
//		Data bits							   8
//		Parity								None
//		Stop bits							   1
//		Flow control						 H/W

// PORTS
//				LEDs on PORTB

//				TWI on PortD
//				SCL (Clock)is on Port D bit 0 (need pullup resistor Set)
//				SDA (Data)is on Port D bit 1 (need pullup resistor Set)

//				USART on PortE
//				USART0 RxD is on Port E bit 0 (Input)
//				USART0 TxD is on Port E bit 1 (Output)


#include <avr/io.h>
#include <avr/interrupt.h>
#include <string.h>

#define CR 0x0D
#define LF 0x0A

// TWI Status codes (MASTER - R/W are with respect to MASTER)
#define TWI_M_START		 	0x08
#define TWI_M_Rep_START	 	0x10
#define TWI_M_SLA_W_ACK	 	0x18
#define TWI_M_SLA_W_NACK	0x20
#define TWI_M_DATA_Tx_ACK	0x28
#define TWI_M_DATA_Tx_NACK	0x30
#define TWI_M_ARB_Lost	 	0x38
#define TWI_M_SLA_R_ACK	 	0x40
#define TWI_M_SLA_R_NACK	0x48
#define TWI_M_DATA_Rx_ACK	0x50
#define TWI_M_DATA_Rx_NACK	0x58

#define TWI_address_Humidity_Temperature_Sensor 0x28	// Default TWI address pre-programmed into the Humidity and Temperature sensor unit

#define TWSR_MASK_5MSB_STATUS_BITS 0xF8		// 5 MSBs are the status code
#define TWINT_FLAG_MASK 0b10000000			// TWCR bit 7 TWINT: TWI Interrupt Flag (low signifies 'TWI busy').

// Configuration and Application-specific function prototypes
void InitialiseGeneral();
void InitialiseTimer3();
void Humidity_Module_MeasurementRequest();
void Humidity_Module_DataFetch();

// TWI related function prototypes
void TWI_MASTER_SETUP();
void TWI_SendSTART();
void TWI_Send_STOP();
void TWI_Send_SLA_W(char cTWI_7bit_RecipientAddress);
void TWI_Send_SLA_R(char cTWI_7bit_RecipientAddress);
void TWI_MASTER_CheckStatusRegisterValueAgainstExpectedValue(unsigned char cExpectedStatusCode);
void WaitFor_TWINT_FLAG();

// USART related function prototypes
void USART_SETUP_9600_BAUD_ASSUME_1MHz_CLOCK();
void USART_TX_SingleByte(unsigned char cByte) ;
void USART_DisplayBanner();
void USART_DisplayPrompt();
void USART_DisplayString(char* cString);


void InitialiseGeneral()
{
	// Ports
	//	PortD TWI interface on 1281 (pins controlled automatically when TWI is enabled)
	//		PortD bit 0  SCL
	//		PortD bit 1  SDA
	// TWI works with a Wire-AND bus thus needs pullup resistors (so bus is at '1' unless pulled low)
	// This code sets pullup resistors on port D TWI bits 0 and 1 (so external resistors are not needed)
	DDRD &=  0b11111100;	// clear bits 1,0 (set direction input)
	PORTD |= 0b00000011;	// Set bits 1,0 (set pullup resistors)

	DDRB = 0xFF;	// PortB	on-board LEDs (Output)
	PORTB = 0xFF;	// Set all LEDs initially off (inverted on the board, so '1' = off)

	sei();	// Enable interrupts at global level set Global Interrupt Enable (I) bit
}



int main( void )
{
	InitialiseGeneral();
	USART_SETUP_9600_BAUD_ASSUME_1MHz_CLOCK();
	
	USART_DisplayString("TWI Master diagnostic");
	
	InitialiseTimer3();
	TWI_MASTER_SETUP();

	while(1)
    {
 	}
}

//**************************************************************
// Humidity and Temperature Application logic		START
//**************************************************************
void Humidity_Module_MeasurementRequest()
{
// Always causes 2 TWINT FLAG episodes whether sensor connected correctly or not
// Verify behaviour with Hyperterminal connected to serial port
	TWI_SendSTART();
	TWI_Send_SLA_W(TWI_address_Humidity_Temperature_Sensor);	// Write to TWI Slave (the sensor)
	TWI_Send_STOP();
}

void Humidity_Module_DataFetch()
{
// Each Data Fetch episode causes 6 TWINT FLAG episodes when working correctly
// Hangs after 2 episodes if sensor not connected correctly
// Verify behaviour with Hyperterminal connected to serial port

// Note that the sensor sends four bytes of data:
// The first pair of bytes contain the 14-bit humidity value right adjusted
// The second pair of bytes contain the 14-bit temperature value left adjusted
	unsigned char Data1, Data2, Data3, Data4;
	TWI_SendSTART();
	TWI_Send_SLA_R(TWI_address_Humidity_Temperature_Sensor);	// Read from specific TWI Slave (the sensor)

	// Receive first data byte, and send ACK to signal more bytes expected
	TWCR = (1<<TWINT)+(1<<TWEN)+(1<<TWEA);	// Clear TWINT flag by writing a '1' to it
									// TWEA is 0, so automatically sends NACK after receiving data byte
									// Signal to H/W this device ready to receive data from prev addressed Slave
	WaitFor_TWINT_FLAG();
	Data1 = TWDR;

	// Receive second data byte, and send ACK to signal more bytes expected
	TWCR = (1<<TWINT)+(1<<TWEN)+(1<<TWEA);	// Clear TWINT flag by writing a '1' to it
									// TWEA is 0, so automatically sends NACK after receiving data byte
									// Signal to H/W this device ready to receive data from prev addressed Slave
	WaitFor_TWINT_FLAG();
	Data2 = TWDR;

	// Receive third data byte, and send ACK to signal more bytes expected
	TWCR = (1<<TWINT)+(1<<TWEN)+(1<<TWEA);	// Clear TWINT flag by writing a '1' to it
									// TWEA is 0, so automatically sends NACK after receiving data byte
									// Signal to H/W this device ready to receive data from prev addressed Slave
	WaitFor_TWINT_FLAG();
	Data3 = TWDR;

	// Receive fourth data byte, and send NACK to signal no more bytes expected
	TWCR = (1<<TWINT)+(1<<TWEN);	// Clear TWINT flag by writing a '1' to it
									// TWEA is 0, so automatically sends NACK after receiving data byte
									// Signal to H/W this device ready to receive data from prev addressed Slave
	WaitFor_TWINT_FLAG();
	Data4 = TWDR;

	TWI_Send_STOP();
	
	long lHraw = ((Data1 & 0b00111111) * 256) + Data2;	// Compose as 16 bit value and masked off 2 MSBs
	long lHumidity = (100 * lHraw) / 16384;
	PORTB = ~lHumidity;									// Display Humidity value on LEDs
	
	long lTraw = ((Data3 * 256) + Data4) /4;			// Compose as 16 bit value and right-adjust 2 places to masked off 2 LSBs
	long lTemperature = ((165 * lTraw) / 16384) - 40;	// Data has a -40 Celcius offset

	unsigned char uHum = lHumidity;
	unsigned char uTem = lTemperature;
	char cHumidityString[100];
	sprintf(cHumidityString,"Humidity %3d RH   Temperature %3d C",uHum,uTem);
	USART_DisplayString(cHumidityString);
}

void InitialiseTimer3()		// Configure to generate an interrupt after a 1 Second interval
{
	TCCR3A = 0b00000000;	// Normal port operation (OC1A, OC1B, OC1C), Clear Timer on 'Compare Match' (CTC) waveform mode
	TCCR3B = 0b00001101;	// CTC waveform mode, use prescaler 1024
	TCCR3C = 0b00000000;
	
	// For 1 MHz clock (with 1024 prescaler) to achieve a 1 second interval:
	// Need to count 1,000,000 clock cycles (but already divided by 1024)
	// So actually need to count to (1000000 / 1024 =) 976 decimal, = 3D0 Hex
	OCR3AH = 0x03; // Output Compare Registers (16 bit) OCR1BH and OCR1BL
	OCR3AL = 0xD0;

	TCNT3H = 0b00000000;	// Timer/Counter count/value registers (16 bit) TCNT3H and TCNT3L
	TCNT3L = 0b00000000;
	TIMSK3 = 0b00000010;	// bit 1 OCIE3A		Use 'Output Compare A Match' Interrupt, i.e. generate an interrupt
							// when the timer reaches the set value (in the OCR3A registers)
}

ISR(TIMER3_COMPA_vect) // TIMER3_Overflow_Handler (Interrupt Handler for Timer 3)
{
	// Send a Measurement request to the sensor
	// This causes the sensor to sample the Humidity and Temperature values
	USART_DisplayString("Humidity MR");
	Humidity_Module_MeasurementRequest();
	
	// Send a Data Fetch request to the sensor
	// This causes the sensor to transmit the measured Humidity and Temperature values onto the TWI bus
	USART_DisplayString("Humidity data");
	Humidity_Module_DataFetch();
}
//**************************************************************
// Humidity and Temperature Application logic		END
//**************************************************************


//**************************************************************
// TWI functionality		START
//**************************************************************
void TWI_MASTER_SETUP()
{
	// Set TWI Bit Rate Register (TWBR)
	// SCL  (SCLf) = CPU Clock frequency (CPUf) / (16 + (2TWBR * TWPS)) - thus fastest SCL is System Clock / 16 with TWBR = 0
	// (where TWPS is one of 1,4,16,64)
	// Slave CPUf must be at least 64 SCLf (from errata sheet, 16 in 8535 manual)
	// In a system in which Microcontrollers run at CPUf = 1MHz, an SCLf of 15KHz is ideal
	// The lower rate allows greater bus length and reduces impacts of noise and bus capacitance
	//	For CPUf = 1MHz, TWBR = 25, TWPS = 1 (bit value 00): gives SCLf = 15151.51 Hz
	//	For CPUf = 1MHz, TWBR = 123, 4^TWPS = 4 (bit value 01): gives SCLf = 1KHz
	//	For CPUf = 1MHz, TWBR = 42, 4^TWPS = 1 (bit value 00): gives SCLf = 10KHz
	//	For CPUf = 1MHz, TWBR = 17, 4^TWPS = 1 (bit value 00): gives SCLf = 20KHz
	//	For CPUf = 1MHz, TWBR = 5, 4^TWPS = 1 (bit value 00): gives SCLf = 38.5KHz
	//	For CPUf = 1MHz, TWBR = 0, 4^TWPS = 1 (bit value 00): gives SCLf = 1MHz / 16 = 62.5KHz
	TWBR = 0;	// SCLf = 62.5KHz

	// Set TWI Status Register (TWSR)
	// bit 7,6,5,4,3 TWS: TWI Status (5 Most Significant bits) (Read only)
	// bit 2 reserved
	// bit 1,0 TWPS1, TWPS0: TWI Prescaler (increases in powers of 4)
	//	 00 = Prescaler 1,	01 = Prescaler 4,	10 = Prescaler 16,	11 = Prescaler 64
	TWSR = 0;	// Use prescaler 1

	// TWI Control Register (TWCR)
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
	TWCR = (1<<TWEA)+(1<<TWEN)+(1<<TWIE);

	// TWI Data Register (TWDR)
	//		For START, 7 MSBs contain Slave Address, LSB = Read (1) / Write (0) flag
	//		No need to initialise this until ready to send a specific data value
}

void TWI_SendSTART() // Implies sender becomes bus Master
{
	TWCR = (1<<TWINT)+(1<<TWEN)+(1<<TWSTA);
	WaitFor_TWINT_FLAG();
	TWI_MASTER_CheckStatusRegisterValueAgainstExpectedValue(TWI_M_START); // Expected Value passed in
}

void TWI_Send_STOP()
{
	TWCR = (1<<TWINT)+(1<<TWEN)+(1<<TWSTO); // Send STOP condition and release the bus
}

void TWI_Send_SLA_W(char cTWI_7bit_RecipientAddress)
{
	TWDR = (cTWI_7bit_RecipientAddress * 2)/*shift left into upper 7 bits*/ +0/*Write*/;	// Slave address  = ssss sss + LSB = Write (0)
	TWCR = (1<<TWINT)+(1<<TWEN);															// Send SLA (Slave Address) + W (Write flag)
	WaitFor_TWINT_FLAG();
	TWI_MASTER_CheckStatusRegisterValueAgainstExpectedValue(TWI_M_SLA_W_ACK); // Expected Value passed in
}

void TWI_Send_SLA_R(char cTWI_7bit_RecipientAddress)
{
	TWDR = (cTWI_7bit_RecipientAddress * 2)/*shift left into upper 7 bits*/ +1/*Read*/;		// Slave address  = ssss sss + LSB = Read (1)
	TWCR = (1<<TWINT)+(1<<TWEN);															// Send SLA (Slave Address) + R (Read flag)
	WaitFor_TWINT_FLAG();
	TWI_MASTER_CheckStatusRegisterValueAgainstExpectedValue(TWI_M_SLA_R_ACK); // Expected Value passed in
}

void TWI_MASTER_CheckStatusRegisterValueAgainstExpectedValue(unsigned char cExpectedStatusCode)
{
	unsigned char TWI_SR;
	TWI_SR = (TWSR & TWSR_MASK_5MSB_STATUS_BITS);  // Get status code from TWSR
	if(cExpectedStatusCode != TWI_SR)
	{	
		TWI_Send_STOP();
	}	
}

void WaitFor_TWINT_FLAG()	// Wait until START condition has been sent
{
	while(!(TWCR & TWINT_FLAG_MASK));  // TWCR bit 7 TWINT: TWI Interrupt Flag (low signifies 'TWI busy').
	// Wait until it is set to a '1' before proceeding
	USART_DisplayString("WaitFor_TWINT_FLAG() completed");
}
//**************************************************************
// TWI functionality		END
//**************************************************************


//**************************************************************
// USART (serial interface) functionality		START
//**************************************************************
void USART_SETUP_9600_BAUD_ASSUME_1MHz_CLOCK()
{
	//UCSR0A � USART Control and Status Register A
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
	// Bits 7:6 � UMSELn1:0 USART Mode Select (00 = Asynchronous)
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
	USART_TX_SingleByte(LF);
	USART_TX_SingleByte(CR);
}
//**************************************************************
// USART (serial interface) functionality		END
//**************************************************************