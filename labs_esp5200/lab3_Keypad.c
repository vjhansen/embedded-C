//*******************************************************************************
// Project      Keypad Demonstration - displays key values 1-9 on a single 7 segment display

// Need to implement it with quad 7 segment 


// Function     Reads keypad (4 column * 4 row matrix)
//              Activates one row at a time, then scans each column within active row
//              Number pressed on keypad is displayed

// Ports            Port C is used for mixed input and output (Keypad scanning)
//                  
//                  Bits 4-7 connect to keypad Rows 
//                  Bits 0-3 connect to keypad Columns

//                  Keypad scanning technique:
//                  Port C pullup resistors are used to pull column pins high
//                  Keypad row pins are set low one at a time
//                  Keypad column pins are checked: 
//                      - normally high
//                      - low signals a key press at corresponding row / column crosspoint

//                  PortA is used for output (7 seg) 

// The 12 Keypad keys form a cross-connect matrix (4 Columns X 4 Rows)
//  
//      Matrix Mapping
//
//      (4 Columns)         C0  C1  C2  C3
//      (4 Rows)        R0  1   2   3   4
//                      R1  5   6   7   8
//                      R2  9   10  11  12
//                      R3  13  14  15  16
//

//      Computing the Key_Value from its row and column positions:
//      
//      A. The key matrix positions are enumerated:
//          '1'  = 0x01, '2'  = 0x02, '3'  = 0x03, '4'  = 0x04
//          '5'  = 0x05, '6'  = 0x06, '7'  = 0x07, '8'  = 0x08
//          '9'  = 0x09, *'10' = 0x0A, *'11' = 0x0B, *'12' = 0x0C
//          *'13' = 0x0D, *'14' = 0x0E, *'15' = 0x0F, *'16' = 0x1F
//      *not shown on 7seg disp

//      B.  The Key_Value is given the initial value of 1, 5, 9 or 13 --> col0 
//          based on the row being scanned.
//      
//      C.  The Key_Value is unchanged if the key detected is in Column 0
//              (values 1, 5, 9, 13)
//          The Key_Value is incremented once if the key detected is in Column 1
//              (values 2, 6, 10, 14)
//          The Key_Value is incremented twice if the key detected is in Column 2
//              (values 3, 7, 11, 15)
//          The Key_Value is incremented twice if the key detected is in Column 3
//              (values 4, 8, 12, 16)

// Keypad connector: 10-pin Port connector pins 0 - 7 are used
//      (The keypad does not require power, the pull-up resistors are sufficient)
//      Bit 0 = C3, Bit 1 = C2, Bit 2 = C1, Bit 3 = C0,
//      Bit 4 = R3, Bit 5 = R2, Bit 6 = R1, Bit 7 = R0


//  Note that the Keyboard handler logic must be inverted
//  I.e. a key press reads as a '0' on the relevant column.
//*****************************************

#include <avr/io.h>
#include <avr/interrupt.h>
#include <util/delay.h>
#include <stdio.h>

#define F_CPU 1000000UL

// Bits 4-7 pulled low depending on row being scanned, bits 0-3 related to the columns (pull-ups) remain high at all times.
// Row0 = bit 7, Row1 = bit 6, Row2 = bit 5, Row3 = bit 4. 
#define ScanKeypadRow0 0b01111111   
#define ScanKeypadRow1 0b10111111
#define ScanKeypadRow2 0b11011111
#define ScanKeypadRow3 0b11101111

// Bits 3:0 connect to keypad Columns
// Col0 = bit 3, Col1 = bit 2, Col2 = bit 1, Col3 = bit0
#define KeypadMaskColumns 0b11110000
#define KeypadMaskColumn0 0b00001000
#define KeypadMaskColumn1 0b00000100
#define KeypadMaskColumn2 0b00000010
#define KeypadMaskColumn3 0b00000001

#define NoKey   0xFF
#define delay_time 2


volatile int KeyPressed;
volatile unsigned char first_digit;
volatile unsigned char second_digit;
volatile unsigned char third_digit;
volatile unsigned char fourth_digit;

void InitialiseGeneral(void);
unsigned char ScanKeypad(void);
unsigned char ScanColumns(unsigned char);
void DisplayNumber(unsigned char);
void DebounceDelay(void);


int main(void) {
    unsigned char KeyValue;
    InitialiseGeneral();
    
    while(1) {
        KeyValue = ScanKeypad();
        switch(PORTB) {
            case 0x08:
                DisplayNumber(first_digit); // Display special chars in different format
                PORTB = 0x04;
                _delay_ms(delay_time);
                break;
            case 0x04:
                DisplayNumber(second_digit);
                PORTB = 0x02;
                _delay_ms(delay_time);
                break;
            case 0x02:
                DisplayNumber(third_digit);
                PORTB = 0x01;
                _delay_ms(delay_time);
                break;
            case 0x01:
                DisplayNumber(fourth_digit);
                PORTB = 0x08;
                _delay_ms(delay_time);
                break;
            default:
                break;
        }
        if(NoKey != KeyValue) {  // key 
            _delay_ms(100);         
            KeyPressed++;
            _delay_ms(100); 
            

            if (KeyPressed == 1) {first_digit = KeyValue;}
            else if (KeyPressed == 2) {second_digit = KeyValue;}
            else if (KeyPressed == 3) {third_digit = KeyValue;}
            else if (KeyPressed == 4) {fourth_digit = KeyValue;}

            if (KeyPressed > 4) {
                KeyPressed = 0;
            }
            DebounceDelay();
        }
    }
}


void DisplayNumber(unsigned char KeyValue) {
    switch (KeyValue) {
        case 0:
            PORTA = 0b10001000;
            break;
        case 1:
            PORTA = 0b10111110;
            break;
        case 2:
            PORTA = 0b00011001;
            break;
        case 3:
            PORTA = 0b00011100;
            break;
        case 4:
            PORTA = 0b00101110;
            break;
        case 5:
            PORTA = 0b01001100;
            break;
        case 6:
            PORTA = 0b01001000;
            break;
        case 7:
            PORTA = 0b10111100;
            break;
        case 8:
            PORTA = 0b00001000;
            break;
        case 9:
            PORTA = 0b00101100;
            break;
        default: // values we have not yet implemented
            PORTA = ~0b00000000; // nothing is displayed
            break;
    }
}

void InitialiseGeneral() {
    DDRA = 0xFF;        // Configure PortA direction for Output
    PORTA = 0xFF;       // Set all LEDs initially off (inverted on the board, so '1' = off) 
    
    // http://www.rjhcoding.com/avrc-bit-manip.php
    // set PINB0, PB1, PB2 and PB3 as outputs for each of the 4 seven segment displays
    DDRB |= (1 << PINB0)|(1 << PINB1)|(1 << PINB2)|(1 << PINB3);
    PORTB = 0x01;
    
    // Port C is used to scan/read the keypad matrix (output on Row bits, read Column bits)
    //  Bit 7 = Y1 (row 0)  Output
    //  Bit 6 = Y2 (row 1)  Output
    //  Bit 5 = Y3 (row 2)  Output
    //  Bit 4 = Y4 (row 3)  Output
    //  Bit 3 = X1 (col 0)  Input 
    //  Bit 2 = X2 (col 1)  Input
    //  Bit 1 = X3 (col 2)  Input
    //  Bit 0 = X4 (col 3)  Input

    DDRC  = 0b11110000; // Port C data direction register (row pins output, column pins input)
    PORTC = 0b00001111; // Set pull-ups on column pins (so they read '1' when no key is pressed)
    
    sei();
}

unsigned char ScanKeypad() {
    unsigned char RowWeight;
    unsigned char KeyValue;

// ScanRow0                 // Row 0 is connected to port bit 7
    RowWeight = 0x01;       // Remember which row is being scanned
    PORTC = ScanKeypadRow0; // Set bit 7 low (Row 0), bits 6,5,4 high (rows 1,2,3)
    KeyValue = ScanColumns(RowWeight);  
    if(NoKey != KeyValue) {
        return KeyValue;
    }
    
// ScanRow1                 // Row 1 is connected to port bit 6
    RowWeight = 0x05;       // Remember which row is being scanned
    PORTC = ScanKeypadRow1; // Set bit 6 low (Row 1), bits 7,5,4 high (rows 0,2,3)
    KeyValue = ScanColumns(RowWeight);  
    if(NoKey != KeyValue) {
        return KeyValue;
    }

// ScanRow2                 // Row 2 is connected to port bit 5
    RowWeight = 0x09;       // Remember which row is being scanned
    PORTC = ScanKeypadRow2; // Set bit 5 low (Row 2), bits 7,6,4 high (rows 0,1,3)
    KeyValue = ScanColumns(RowWeight);  
    if(NoKey != KeyValue) {
        return KeyValue;
    }

// ScanRow3                 // Row 3 is connected to port bit 4
    RowWeight = 0x0D;       // Remember which row is being scanned = S13
    PORTC = ScanKeypadRow3; // Set bit 4 low (Row 3), bits 7,6,5 high (rows 0,1,2)
    KeyValue = ScanColumns(RowWeight);  
    return KeyValue;
}

unsigned char ScanColumns(unsigned char RowWeight) {
    // Read bits 7, 6, 5, 4 as high, as only interested in any low values in bits 3, 2, 1, 0
    unsigned char ColumnPinsValue; 
    ColumnPinsValue = PINC | KeypadMaskColumns; // '0' in any column position means key pressed
    ColumnPinsValue = ~ColumnPinsValue;         // '1' in any column position means key pressed

    if(KeypadMaskColumn0 == (ColumnPinsValue & KeypadMaskColumn0)) {
        return RowWeight;       // Indicates current row + column 0
    }
    if(KeypadMaskColumn1 == (ColumnPinsValue & KeypadMaskColumn1)) {
        return RowWeight + 1;   // Indicates current row + column 1
    }
    if(KeypadMaskColumn2 == (ColumnPinsValue & KeypadMaskColumn2)) {
        return RowWeight + 2;   // Indicates current row + column 2
    }
    if(KeypadMaskColumn3 == (ColumnPinsValue & KeypadMaskColumn3)) {
        return RowWeight + 3;   // Indicates current row + column 3
    }
    return NoKey;   // Indicate no key was pressed
}

void DebounceDelay() {
    for(int i = 0; i < 50; i++) {
        for(int j = 0; j < 255; j++);
    }
}
