/*
  keypad.h
  Updated: Victor Hansen, 06.11.20

  Based on: Lab 3 - keypad
  and KEYPAD_LEDs_1281_C by Richard Anthony - 17th October 2013
  -------------------------------
  Keypad    <-> Arduino Mega
  P0 is the top-most pin (almost on the same row as S1, S2 ..)
  Px is the bottom-most pin (almost on the same row as S9, S10 ..)
  - - - - - - - - - - - -
  P0  <-> PC0
  P1  <-> PC1
  P2  <-> PC2
  P3  <-> PC3
  P4  <-> PC7
  P5  <-> PC6
  P6  <-> PC5
  P7  <-> PC4
  - - - - - - - - - - - -
*/

#include <avr/io.h>

/* Bits 4-7 pulled low depending on row being scanned, 
   bits 0-3 related to the columns (pull-ups) remain high at all times. */
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

#define NoKey       0xFF
#define delay_time  2
#define NUM_rows    4

unsigned char ScanKeypad(void);
unsigned char ScanColumns(unsigned char);

unsigned char ScanKeypad()
{
    unsigned char RowWeight, KeyValue;
    unsigned char row_weight[NUM_rows] = {0x01, 0x05, 0x09, 0x0D}; // S1, S5, S9, S13
    unsigned char keypad_row[NUM_rows] = {ScanKeypadRow0, ScanKeypadRow1, ScanKeypadRow2, ScanKeypadRow3};
    
    for (unsigned char i = 0; i < NUM_rows; i++) 
    {
        RowWeight = row_weight[i];  // Remember which row is being scanned
        PORTC = keypad_row[i]; // Set Row 'i' low, and the other Rows high
        KeyValue = ScanColumns(RowWeight);
        if(NoKey != KeyValue) {
            return KeyValue;
        }
    }
}

// by R. Anthony
unsigned char ScanColumns(unsigned char RowWeight)
{
    // Read bits 7, 6, 5, 4 as high, as only interested in any low values in bits 3, 2, 1, 0
    unsigned char ColumnPinsValue;
    ColumnPinsValue = PINC | KeypadMaskColumns; // '0' in any column position means key pressed
    ColumnPinsValue = ~ColumnPinsValue;         // '1' in any column position means key pressed
    if(KeypadMaskColumn0 == (ColumnPinsValue & KeypadMaskColumn0))
    {
        return RowWeight;       // Indicates current row + column 0
    }
    if(KeypadMaskColumn1 == (ColumnPinsValue & KeypadMaskColumn1))
    {
        return RowWeight + 1;   // Indicates current row + column 1
    }
    if(KeypadMaskColumn2 == (ColumnPinsValue & KeypadMaskColumn2))
    {
        return RowWeight + 2;   // Indicates current row + column 2
    }
    if(KeypadMaskColumn3 == (ColumnPinsValue & KeypadMaskColumn3))
    {
        return RowWeight + 3;   // Indicates current row + column 3
    }
    return NoKey;   // else - no key was pressed
}
