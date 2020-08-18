;** Project 		RFID Reader Library (assumes LCD output)
;** Target 			ATmega8535 on STK200
;** Program			RFID_Reader.inc	
;** Author			Richard Anthony
;** Date			16th January 2010 	

;** Main Function	Use of RFID reader ID-12
;**					Configures RFID reader for ASCII output data format
;**					Uses the TTL data output (D0 - this is 'inverted' (Normally High) but actually
;**					works directly with the USART 'as is'
;**					The ID-12 also has a CMOS data output (D1) 'non-inverted' (Normally Low)
				
;** USART			Used to receive data from the ID-12 RFID reader device
;**					Baud rate 9600, Asynchronous mode

;**	I/O				RFID D0 is connected to the USART receive (RXD)
;**					RXD is the alternate function of PORT D, bit 0; set for input in application code
;********************************************************************

		// Port D bit 0 is used by the RFID reader
		.equ RFID_DDR		=	DDRD
		.equ RFID_PORT		=	PIND
		.equ TTL_DATA 		=	0	// TTL (data is inverted)
		
		.include "m8535def.inc" ;Definitions for ATmega8535

		.def	RFID_BYTE_COUNT	 = r10	//Number of bytes received so far, Also data buffer length
		.def	RFID_DATA_BYTE	 = r21
		.def	RFID_BUFFER_LOW  = r28	//(Y lower)
		.def	RFID_BUFFER_HIGH = r29	//(Y upper)

		;RFID buffer in RAM
		.equ	RFID_BUFFER_ADDRESS = 0x0100

		; Constants for RFID code validation
		.equ	RFID_ASCII_CR=$0D	;ASCII carriage return
		.equ	RFID_ASCII_LF=$0A	;ASCII line feed
		.equ	RFID_STX=$02 		;Start of transmission
		.equ	RFID_ETX=$03 		;End of transmission
		.equ	RFID_CODE_LENGTH=16 ;Number of bytes in correctly formatted code
		
		.cseg		;CODE segment

RFID_INITIALISATION:
		rcall USART_SETUP
		rcall RFID_BUFFER_INITIALISATION
		rcall RFID_READ_INITIALISATION
		ret

RFID_READ_INITIALISATION:
		rcall RFID_BUFFER_INITIALISATION
		clr RFID_BYTE_COUNT
		ret

RFID_BUFFER_INITIALISATION:
		// Buffer access uses 'Indirect with Displacement' Data Addressing 
		ldi RFID_BUFFER_HIGH, HIGH(RFID_BUFFER_ADDRESS)	//register Y; r29 = high
		ldi RFID_BUFFER_LOW, LOW(RFID_BUFFER_ADDRESS)	//register Y; r28 = low
		ret

RFID_STORE_BYTE_IN_BUFFER:
		rcall RFID_BUFFER_INITIALISATION
		add RFID_BUFFER_LOW, RFID_BYTE_COUNT
		st Y, RFID_DATA_BYTE		
		inc RFID_BYTE_COUNT
		ret

RFID_READ_VALIDATE: // Called when RFID byte read count >= 16
		ldi temp, RFID_CODE_LENGTH
		cp RFID_BYTE_COUNT, temp
		breq RFID_CODE_LENGTH_CORRECT
		rjmp RFID_ERROR			// Code too long
		
RFID_CODE_LENGTH_CORRECT:		
		rcall RFID_BUFFER_INITIALISATION
		ld RFID_DATA_BYTE, Y		
		cpi RFID_DATA_BYTE, RFID_STX  	// Check first byte is STX
		brne RFID_ERROR			// Code incorrect

		ldi temp, 13			// 14th byte
		add RFID_BUFFER_LOW, temp
		ld RFID_DATA_BYTE, Y		
		cpi RFID_DATA_BYTE, RFID_ASCII_CR // Check 14th byte is CR
		brne RFID_ERROR			// Code incorrect

		inc RFID_BUFFER_LOW
		ld RFID_DATA_BYTE, Y		
		cpi RFID_DATA_BYTE, RFID_ASCII_LF  // Check 15th byte is LF
		brne RFID_ERROR			// Code incorrect

		inc RFID_BUFFER_LOW
		ld RFID_DATA_BYTE, Y		
		cpi RFID_DATA_BYTE, RFID_ETX  	// Check 16th (last) byte is ETX
		brne RFID_ERROR			// Code incorrect

		// ALSO - check the XOR checksum
		// I curently have only one Tag compatible with the RFID reader, and for this tag
		// ASCII data "", the XOR checksum comes out 1 bit wrong, whichever way it is calculated
		// Need more compatible tags before this behaviour can be investigated further, and then
		// a checksum checker can be codifed and added here. 

		ldi temp, 0xC3
		com temp
		out portb, temp

		rcall LCD_GOOD
		rcall LCD_DATA
		rcall RFID_DISPLAY_TAG_ID_ON_LCD
		rcall LCD_CHECKSUM
		rcall RFID_DISPLAY_TAG_CHECKSUM_ON_LCD
		ret

RFID_ERROR:
// here when too many bytes received, or validation fails
		ldi temp, 0x55
		com temp
		out portb, temp
		rcall LCD_ERROR
		rcall RFID_DISPLAY_TAG_FULL_DATA_ON_LCD
		ret


RFID_DISPLAY_TAG_FULL_DATA_ON_LCD:
		ldi	LCDreg, 0x40  	; Move the LCD cursor position to 2nd line, 1st column
		rcall lcd_SetCursor
		rcall RFID_BUFFER_INITIALISATION
		push RFID_BUFFER_HIGH		// Pass arguments via stack
		push RFID_BUFFER_LOW
		push RFID_BYTE_COUNT
		rcall ReadAndDisplayMessageFromDATAmemoryBuffer
		ret

RFID_DISPLAY_TAG_ID_ON_LCD:
		ldi	LCDreg, 0x46  	; Move the LCD cursor position to 2nd line, 7th column
		rcall lcd_SetCursor
		rcall RFID_BUFFER_INITIALISATION
		push RFID_BUFFER_HIGH		// Pass arguments via stack
		inc RFID_BUFFER_LOW			// Skip the STX character
		push RFID_BUFFER_LOW
		ldi temp, 10				// Only want the ten DATA characters
		push temp
		rcall ReadAndDisplayMessageFromDATAmemoryBuffer
		ret

RFID_DISPLAY_TAG_CHECKSUM_ON_LCD:
		ldi	LCDreg, 0x0E  	; Move the LCD cursor position to 1st line, 14th column
		rcall lcd_SetCursor
		rcall RFID_BUFFER_INITIALISATION
		push RFID_BUFFER_HIGH		// Pass arguments via stack
		ldi temp, 11				// 12th byte read from the RFID tag
		add RFID_BUFFER_LOW, temp
		push RFID_BUFFER_LOW
		ldi temp, 2					// Only want the two CHECKSUM characters
		push temp
		rcall ReadAndDisplayMessageFromDATAmemoryBuffer
		ret

RFID_TAG_COMPARE:
		// Compare the 10-byte TAG ID of the most-recently read tag in DATA memory, with
		// a statically-coded reference TAG ID held in PROGRAM memory
		
		// Point Y register to the TAG ID in DATA memory
		rcall RFID_BUFFER_INITIALISATION
		inc RFID_BUFFER_LOW			// Skip the STX character
		
		// Point Z register to the REFERENCE TAG ID in PROGRAM memory
		ldi	ZH,HIGH(2*REFERENCE_TAG_ID_CORRECT) ; The text is located in the program memory
		ldi	ZL,LOW(2*REFERENCE_TAG_ID_CORRECT) ;  and the lpm command (and Z register) is used to read it

		// Loop 10 times, each time incrementing BOTH indexes
		// Break the loop if the bytes in each index are different: signals no match
		// If all 10 bytes match their counterparts, the TAG matches the reference
		ldi temp, 10
		mov RFID_BYTE_COUNT,temp
RFID_TAG_COMPARE_LOOP:		
		ld RFID_DATA_BYTE, Y		; Read DATA memory (from current Y register location)	
		lpm 						; Read PROGRAM memory (from current Z register location)
		cpse RFID_DATA_BYTE, rlpm	; Compare current data byte with its reference counterpart
		rjmp RFID_TAG_COMPARE_NO_MATCH
		adiw Y,1
		adiw Z,1
		dec RFID_BYTE_COUNT
		brne RFID_TAG_COMPARE_LOOP

RFID_TAG_COMPARE_MATCH:		
		rcall LCD_MATCH
		ret

RFID_TAG_COMPARE_NO_MATCH:
		rcall LCD_NO_MATCH
		ret


// ***** USART configuration subroutines *****
USART_SETUP:
// UCSRA - USART Control and Status Register A
;bit 7 RXC Receive Complete (flag)
;bit 6 TXC Transmit Complete (flag)
;bit 5 UDRE Data Register Empty (flag)
;bit 4 FE Frame Error (flag) - programmatically clear this when writing to UCSRA
;bit 3 DOR Data OverRun (flag)
;bit 2 PE Parity Error
;bit 1 UX2 Double the USART TX speed (but also depends on value loaded into the Baud Rate Registers)
;bit 0 MPCM Multi-Processor Communication Mode
		ldi temp, 0b00000010  // U2X (Double the USART transmission speed, done to reduce clocking error,
							  // See 8535 datasheet, page 170, table 69
		out UCSRA, temp

// UCSRB - USART Control and Status Register B
;bit 7 RXCIE Receive Complete Interrupt Enable
;bit 6 TXCIE Transmit Complete Interrupt Enable
;bit 5 UDRIE Data Register Empty Interrupt Enable
;bit 4 RXEN Receiver Enable
;bit 3 TXEN Transmitter Enable
;bit 2 UCSZ2 Character Size (see also UCSZ1:0 in UCSRC)
;0 = 5,6,7 or 8-bit data
;1 = 9-bit data
;bit 1 RXB8 RX Data bit 8 (only for 9-bit data)
;bit 0 TXB8 TX Data bit 8 (only for 9-bit data)
		ldi temp, 0b10010000  // RX Complete Int Enable, RX Enable, 8-bit data
		out UCSRB, temp

// UCSRC - USART Control and Status Register C
// *** This register shares the same I/O location as UBRRH *** 
;bit 7 URSEL Register SELect, controls which of the following two registers are selected:
;0 (during a write operation) selects UBRRH
;1 (during a write operation) selects UCSRC (i.e. THIS register)
;bit 6 UMSEL USART Mode SELect
;0 = Asynchronous
;1 = Synchronous (XCK appears on PortB bit 0 if set for output)
;Note, in synchronous mode the Data Direction register for the XCK pin, i.e. PortB (DDRB) bit 0
;controls whether the clock source is internal or external
;bit 5:4 UPM1:0 Parity Mode
;00 Disabled
;10 Even parity
;11 Odd parity
;bit 3 USBS Stop Bit Select
;0 = 1 stop bit
;1 = 2 stop bits
;bit 2:1 UCSZ1:0 Character Size (see also UCSZ2 in UCSRB)
;00 = 5-bit data (UCSZ2 = 0)
;01 = 6-bit data (UCSZ2 = 0)
;10 = 7-bit data (UCSZ2 = 0)
;11 = 8-bit data (UCSZ2 = 0)
;11 = 9-bit data (UCSZ2 = 1)
;bit 0 UCPOL Clock POLarity
;0 Rising XCK edge
;1 Falling XCK edge
		ldi temp, 0b10000111 //Select UCSRC, Asynchronous, No Parity, 1 stop ,8-bit data, Falling XCK edge
		out UCSRC, temp
//		sbi DDRB, USART_XCK // Set XCK pin for output (sets the synchronous mode clock to internal)
							// Only applies when synchronous mode is used

// UBRR - USART Baud Rate Register (16-bit register, comprising UBRRH and UBRRL) 
// UBRRH - USART Baud Rate Register (High byte) 
// *** This register shares the same I/O location as UCSRC *** 
;bit 15 URSEL Register SELect, controls which of the following two registers are selected:
;0 (during a write operation) selects UBRRH (i.e. THIS register)
;1 (during a write operation) selects UCSRC 
;bit 14:12 reserved
;bit 11:8 (upper 4 bits of 12) USART Baud Rate Register 

// UBRRL - USART Baud Rate Register (Low byte) 
;bit 7:0 (lower 8 bits of 12) USART Baud Rate Register 

// Baud Rate setting, NOTES: 1 Value placed into UBRR depends on value of U2X bit in UCSRA
//								(but only when Asynchronous mode is used)
//							 2 See tables of values (8535 datasheet) page 170, tables 69-72 
		ldi temp, 0     // URSEL = 0, 9600 baud, UBRR = 12, and  U2X must be set to '1' in UCSRA
		out UBRRH,temp
		ldi temp, 12
		out UBRRL,temp
		ret

USART_RX_Complete_Handler:	// USART Receive-Complete Interrupt Handler
		cli
		in RFID_DATA_BYTE, UDR	// Get received data from buffer
		rcall RFID_STORE_BYTE_IN_BUFFER

		// when 16 bytes have been received, call Validation subroutine
		ldi temp, RFID_CODE_LENGTH
		cp RFID_BYTE_COUNT, temp
		brlo USART_RX_Complete_Handler_Exit
		rcall RFID_READ_VALIDATE
		rcall RFID_READ_INITIALISATION

USART_RX_Complete_Handler_Exit:
		reti  // Incorporates 'sei'
// ***** END of USART subroutines *****



