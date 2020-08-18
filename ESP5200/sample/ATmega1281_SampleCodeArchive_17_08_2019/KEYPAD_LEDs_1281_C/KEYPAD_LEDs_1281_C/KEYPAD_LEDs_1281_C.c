//*******************************************************************************
// Project 		Keypad Demonstration - displays key value on LEDs (Embedded C) 
// Target 		ATMEL ATmega1281 micro-controller on STK300 board
// Program		KEYPAD_LEDs_1281_C.c
// Author		Richard Anthony
// Date			17th October 2013 (ATmega8535 version 8th October 2011)

// Function		Reads keypad (3 column * 4 row matrix)
// 				Activates one row at a time, then scans each column within active row
//  				Number pressed on keypad is displayed in Binary on LEDs

// Ports	  		Port C is used for mixed input and output (Keypad scanning)
//					
//					Bit  7 is not used
//					Bits 3-6 connect to keypad Rows 
//					Bits 0-2 connect to keypad Columns

//					Keypad scanning technique:
//					Port C pullup resistors are used to pull column pins high
//					Keypad row pins are set low one at a time
//					Keypad column pins are checked: 
//						- normally high
//						- low signals a key press at corresponding row / column crosspoint

//					Port B is used for output (on-board LEDs) 

// The 12 Keypad keys form a cross-connect matrix (3 Columns X 4 Rows)
//	
//		Matrix Mapping
//
// 		(3 Columns)			C0 	C1	C2 
//		(4 Rows)		R0	1	2	3
//						R1	4	5	6
//						R2	7	8	9
//						R3	*	0	#
//

//		Computing the Key_Value from its row and column positions:
//		
//		A. The key matrix positions are enumerated 1(0x01) - 12(0x0C)
//			'1' = 0x01, '2' = 0x02, '3' = 0x03
//			'4' = 0x04, '5' = 0x05, '6' = 0x06
//			'7' = 0x07, '8' = 0x08, '9' = 0x09
//			'*' = 0x0A, '0' = 0x0B, '#' = 0x0C
//		
//		B.	The Key_Value is given the initial value of 1,4,7 or 10 (0x0A) 
//			based on the row being scanned.
//		
//		C. The Key_Value is unchanged if the key detected is in Column 0
//			(values 1, 4, 7, 10)
//			The Key_Value is incremented once if the key detected is in Column 1
//			(values 2, 5, 8, 11)
//			The Key_Value is incremented twice if the key detected is in Column 2
//			(values 3, 6, 9, 12)


// Keypad connector: 10-pin Port connector pins 0 - 6 are used
//		(The keypad does not require power, the pullup resistors are sufficient)
//		Bit 0 = C2, Bit 1 = C1, Bit 2 = C0
//		Bit 3 = R3, Bit 4 = R2, Bit 5 = R1, Bit 6 = R0
	

//	Note that the Keyboard handler logic must be inverted
//  I.e. a key press reads as a '0' on the relevant column.
//*****************************************

#include <avr/io.h>
#include <avr/interrupt.h>

#define ScanKeypadRow0 0b00111111	// Bits 4-7 pulled low depending on row being scanned, bits 0-2 (pullups) remain high at all times
#define ScanKeypadRow1 0b01011111
#define ScanKeypadRow2 0b01101111
#define ScanKeypadRow3 0b01110111

#define KeypadMaskColumns 0b11111000
#define KeypadMaskColumn0 0b00000100
#define KeypadMaskColumn1 0b00000010
#define KeypadMaskColumn2 0b00000001

#define Star	0x0A
#define Zero	0x0B
#define Hash	0x0C
#define NoKey	0xFF

void InitialiseGeneral(void);
unsigned char ScanKeypad(void);
unsigned char ScanColumns(unsigned char);
void DisplayKeyValue(unsigned char);
void DebounceDelay(void);

int main( void )
{
	unsigned char KeyValue;
	InitialiseGeneral();

    while(1)
    {
		KeyValue = ScanKeypad();
		
		if(NoKey != KeyValue)
		{
			//PORTB = ~KeyValue;		// Display actual value derived from matrix position
			DisplayKeyValue(KeyValue);	// Display special chars in different format
			DebounceDelay();
		}
    }
}

void InitialiseGeneral()
{
	DDRB = 0xFF;			// Configure PortB direction for Output
	PORTB = 0xFF;			// Set all LEDs initially off (inverted on the board, so '1' = off)	
	
	// Port C is used to scan/read the keypad matrix (output on Row bits, read Column bits)
	//	Bit 7 not used		Output
	//	Bit 6 = Y1 (row 0)	Output
	//	Bit 5 = Y2 (row 1)	Output
	//	Bit 4 = Y3 (row 2)	Output
	//	Bit 3 = Y4 (row 3)	Output
	//	Bit 2 = X1 (col 0)	Input 
	//	Bit 1 = X2 (col 1)	Input
	//	Bit 0 = X3 (col 2)	Input
	DDRC = 0b11111000;	// Port C data direction register (row pins output, column pins input)
	PORTC= 0b00000111;	// Set pullups on column pins (so they read '1' when no key is pressed)
	
	sei();
}

unsigned char ScanKeypad()
{
	unsigned char RowWeight;
	unsigned char KeyValue;

// ScanRow0					// Row 0 is connected to port bit 6
	RowWeight = 0x01;		// Remember which row is being scanned
	PORTC = ScanKeypadRow0;	// Set bit 6 low (Row 0), bits 5,4,3 high (rows 1,2,3)
	KeyValue = ScanColumns(RowWeight);	
	if(NoKey != KeyValue)
	{
		return KeyValue;
	}
	
// ScanRow1					// Row 1 is connected to port bit 5
	RowWeight = 0x04;		// Remember which row is being scanned
	PORTC = ScanKeypadRow1;	// Set bit 5 low (Row 1), bits 6,4,3 high (rows 0,2,3)
	KeyValue = ScanColumns(RowWeight);	
	if(NoKey != KeyValue)
	{
		return KeyValue;
	}

// ScanRow2					// Row 2 is connected to port bit 4
	RowWeight = 0x07;		// Remember which row is being scanned
	PORTC = ScanKeypadRow2;	// Set bit 4 low (Row 2), bits 6,5,3 high (rows 0,1,3)
	KeyValue = ScanColumns(RowWeight);	
	if(NoKey != KeyValue)
	{
		return KeyValue;
	}

// ScanRow3					// Row 3 is connected to port bit 3
	RowWeight = 0x0A;		// Remember which row is being scanned
	PORTC = ScanKeypadRow3;	// Set bit 3 low (Row 3), bits 6,5,4 high (rows 0,1,2)
	KeyValue = ScanColumns(RowWeight);	
	return KeyValue;
}

unsigned char ScanColumns(unsigned char RowWeight)
{
	// Read bits 7,6,5,4,3 as high, as only interested in any low values in bits 2,1,0
	unsigned char ColumnPinsValue; 
	ColumnPinsValue = PINC | KeypadMaskColumns; // '0' in any column position means key pressed
	ColumnPinsValue = ~ColumnPinsValue;			// '1' in any column position means key pressed

	if(KeypadMaskColumn0 == (ColumnPinsValue & KeypadMaskColumn0))
	{
		return RowWeight;		// Indicates current row + column 0
	}
	
	if(KeypadMaskColumn1 == (ColumnPinsValue & KeypadMaskColumn1))
	{
		return RowWeight + 1;	// Indicates current row + column 1
	}

	if(KeypadMaskColumn2 == (ColumnPinsValue & KeypadMaskColumn2))
	{
		return RowWeight + 2;	// Indicates current row + column 2
	}
	
	return NoKey;	// Indicate no key was pressed
}

void DisplayKeyValue(unsigned char KeyValue)
{
	if(Star == KeyValue)
	{
		PORTB = ~0x2A;		// Special key '*' was pressed
		return;
	} 
	
	if(Zero == KeyValue)
	{
		PORTB = ~0x4B;		// Special key '0' was pressed
		return;
	} 
	
	if(Hash == KeyValue)
	{
		PORTB = ~0x8C;		// Special key '#' was pressed
		return;
	} 
		
	PORTB = ~KeyValue;		// Regular numeric key was pressed
}

void DebounceDelay()	// This delay is needed because after pressing a key, the mechanical switch mechanism tends
						// to bounce, possibly leading to many key presses being falsely detected. The debounce delay 
						// makes the program wait long enough for the bouncing to stop before reading the keypad again.	
{
	for(int i = 0; i < 50; i++)
	{
		for(int j = 0; j < 255; j++);
	}
}