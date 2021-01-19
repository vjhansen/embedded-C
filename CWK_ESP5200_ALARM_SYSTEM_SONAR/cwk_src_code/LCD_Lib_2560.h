
// Minor updates: Victor Hansen, 06.11.2020

/*
  LCD_LibraryFunctions_2560.h, 
  Richard Anthony, 16th October 2016
*/
  
/*
  -------------------------------
  LCD     <-> Arduino Mega
  -------------------------------
  VSS  <-> GND
  VDD  <-> 5V
  VO   <-> Potentiometer (contrast control)
  RS   <-> PG0
  RW   <-> PG1
  E    <-> PG2
  D0   <-> PA0
  D1   <-> PA1
  D2   <-> PA2
  D3   <-> PA3
  D4   <-> PA4
  D5   <-> PA5
  D6   <-> PA6
  D7   <-> PA7
  A    <-> 5V via 1kohm (A = Backlight LED anode)
  K    <-> GND  (K = Backlight LED cathode)
  ----------------------------
*/

#include <stdbool.h>
#include <string.h>

#define LCD_DisplayWidth_CHARS  16
#define LCD_anodePin            PK3

// Function declarations
void LCD_Write_CommandOrData(bool bCommand /*true = Command, false = Data*/, unsigned char DataOrCommand_Value);
void LCD_Wait();

/* bTwoLine: false = 1-line mode, true = 2-line mode*/
/* bLargeFont: false = 5*8pixels, true = 5*11 pixels*/
void LCD_Initilise(bool bTwoLine, bool bLargeFont);

// Turn the LCD display ON / OFF
void LCD_Display_ON_OFF(bool bDisplayON /*true=ON, false=OFF*/, bool bCursorON, bool bCursorPositionON); 

void LCD_Clear();
void LCD_Home();
void LCD_WriteChar(unsigned char cValue);
void LCD_ShiftDisplay(bool bShiftDisplayON /*true=On, false=OFF*/, bool bDirectionRight /*true=SHR, false=SHL*/);

/* iRowPosition: 0 for top row, 1 for bottom row */
void LCD_SetCursorPosition(unsigned char iColumnPosition /*0-40 */, unsigned char iRowPosition);

void LCD_WriteString(char Text[]);

void LCD_Write_CommandOrData(bool bCommand /*true=Command, false=Data*/, unsigned char DataOrCommand_Value)
{
    LCD_Wait(); // Wait if LCD device is busy
    
    // The access sequence is as follows:
    // 1. Set command lines as necessary:
    if(true == bCommand)
    {   // Register Select LOW, and R/W direction LOW
        PORTG &= ~(1<<PG1 | 1<<PG0);  // Clear Read(H)/Write(L) (PG1), Clear Register Select for command mode (PG0)
    }
    else /*Data*/
    {   // Register Select HIGH, and R/W direction LOW
        PORTG |= (1 << PG0);  // Set Register Select HIGH for data mode (PortG bit0)
        PORTG &= ~(1 << PG1); // Clear Read(H)/Write(L) (PortG bit1)
    }
    // 2. Set Enable High
    // 3. Write data or command value to PORTA
    // 4. Set Enable Low
    PORTG |= (1 << PG2);  // Set LCD Enable (PortG bit2)
    DDRA = 0xFF;                    // Configure PortA direction for Output
    PORTA = DataOrCommand_Value;    // Write combined command value to port A
    PORTG &= ~(1 << PG2); // Clear LCD Enable (PortG bit2)
}

void LCD_Wait()     // Check if the LCD device is busy, if so wait
{                   // Busy flag is mapped to data bit 6, so read as port A bit 6
    PORTG &= ~(1<<PG0); // Clear Register Select for command mode (PortG bit0)
    PORTG |= (1<<PG1);  // Set Read(H)/Write(L) (PortG bit1)
    DDRA = 0x00;        // Configure PortA direction for Input (so busy flag can be read)
    
    unsigned char PINA_value = 0;
    while(PINA_value & 0x80);   // Wait here until busy flag is cleared
    {
        PORTG |= (1<<PG2);  // Set LCD Enable (PortG bit2)
        PINA_value = PINA;
        PORTG &= ~(1<<PG2); // Clear LCD Enable (PortG bit2)
    }
}

/* bTwoLine: false = 1 line mode, true =  2 line mode */
/* bLargeFont: false = 5*8pixels, true = 5*11 pixels */
void LCD_Initilise(bool bTwoLine, bool bLargeFont)
{   // Note, in 2-line mode must use 5*8 pixels font
    // Set Port A and Port G for output
    DDRG  = 0xFF;   // Configure PortG direction for Output
    PORTG = 0x00;   // Clear port G
    DDRA  = 0xFF;   // Configure PortA direction for Output
    PORTA = 0x00;   // Clear port A

    unsigned char Command_value = 0b00110000;  // bit 5 'Function Set' command, bit 4 sets 8-bit interface mode
    if(true == bTwoLine)
    {
        Command_value |= 0b00001000;    // bit 3 high = 2-line mode (low = 1 line mode)
    }
    else {  // One-line mode
        if(true == bLargeFont)
        {
            // Large font (nested because can only use large font in one-line mode)
            Command_value |= 0b00000100;  // bit 2 high = large font mode 5*11 pixels (low = small font 5*8pixels)
        }
    }
    LCD_Write_CommandOrData(true /*true = Command, false = Data*/, Command_value);
}
// Turn the LCD display ON / OFF
void LCD_Display_ON_OFF(bool bDisplayON /*true = ON, false = OFF*/, bool bCursorON, bool bCursorPositionON)
{
    if(true == bDisplayON) {
        if(true == bCursorON) {
            if(true == bCursorPositionON)
            {
                // 'Display ON/OFF' function command, Display ON, Cursor ON, Cursor POSITION ON
                LCD_Write_CommandOrData(true /*true = Command, false = Data*/, 0b00001111);
            }
            else
            {
                // 'Display ON/OFF' function command, Display ON, Cursor ON, Cursor POSITION OFF
                LCD_Write_CommandOrData(true /*true = Command, false = Data*/, 0b00001110);
            }
        }
        else { /*Cursor OFF*/
            if(true == bCursorPositionON)
            {
                // 'Display ON/OFF' function command, Display ON, Cursor OFF, Cursor POSITION ON
                LCD_Write_CommandOrData(true /*true = Command, false = Data*/, 0b00001101);
            }
            else
            {
                // 'Display ON/OFF' function command, Display ON, Cursor OFF, Cursor POSITION OFF
                LCD_Write_CommandOrData(true /*true = Command, false = Data*/, 0b00001100);
            }
        }
    }
    else
    {
        // 'Display ON/OFF' function command, Display OFF, Cursor OFF, Cursor POSITION OFF
        LCD_Write_CommandOrData(true /*true = Command, false = Data*/, 0b00001000);
    }
}

void LCD_Clear()  // Clear the LCD display
{
    LCD_Write_CommandOrData(true /*true=Command, false=Data*/, 0x01);
    _delay_ms(2); // Enforce delay for specific LCD operations to complete
}

void LCD_Home() // Set the cursor to the 'home' position
{
    LCD_Write_CommandOrData(true /*true = Command, false = Data*/, 0x02);
    _delay_ms(2);   // Enforce delay for specific LCD operations to complete
}

void LCD_WriteChar(unsigned char cValue)
{   // Write character in cValue to the display at current cursor position (position is incremented after write)
    LCD_Write_CommandOrData(false /*true=Command, false=Data*/, cValue);
}

void LCD_ShiftDisplay(bool bShiftDisplayON /*true=On, false=OFF*/, bool bDirectionRight /*true=SHR, false=SHL*/)
{
    if(true == bShiftDisplayON) {
        if(true == bDirectionRight)
        {
            LCD_Write_CommandOrData(true /*true = Command, false = Data*/, 0b00000101);
        }
        else /*shift display left*/
        {
            LCD_Write_CommandOrData(true /*true = Command, false = Data*/, 0b00000111);
        }
    }
    else /*ShiftDisplay is OFF*/
    {
        LCD_Write_CommandOrData(true /*true = Command, false = Data*/, 0b00000100);     
    } 
    if(true == bShiftDisplayON) {
        if(true == bDirectionRight) {
            LCD_Write_CommandOrData(true /*true = Command, false = Data*/, 0b00000101);
        }
        else { /*shift display left*/
            LCD_Write_CommandOrData(true /*true = Command, false = Data*/, 0b00000111);
        }
    }
    else { /*ShiftDisplay is OFF*/
        LCD_Write_CommandOrData(true /*true = Command, false = Data*/, 0x04);
    }
}

/* iColumnPosition: 0 - 40 */
/* iRowPosition: 0 for top row, 1 for bottom row */
void LCD_SetCursorPosition(unsigned char iColumnPosition, unsigned char iRowPosition)
{
    // Cursor position is achieved by repeatedly shifting from the home position.
    // In 2-line mode, the beginning of the second line is the 41st position (from home position)
    unsigned char iTargetPosition = (40*iRowPosition) + iColumnPosition;
    LCD_Home();
    for(unsigned char iPos = 0; iPos<iTargetPosition; iPos++)
    {
        LCD_Write_CommandOrData(true /*true = Command, false = Data*/, 0b00010100); // Shift cursor left 1 place
    }
}

void LCD_WriteString(char Text[])
{
    for(unsigned char iIndex = 0; iIndex<strlen(Text); iIndex++)
    {
        LCD_WriteChar(Text[iIndex]);
    }
}
