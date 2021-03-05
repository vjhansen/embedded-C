/*
Library for LCD EA DOGM163 (4-bit/5V) 16x2.

uC: ATmega16.

*/


#define LCD_PORT	PORTC   
#define PIN_RW		PC0
#define PIN_RS 		PC1
#define PIN_ENABLE 	PC2	

// - Defining instructions for the LCD.
#define CLEAR_DISP    			0b00000001	// - Clear display.
#define FUNCTION_SET_INIT_0		0b00110000  	// - 8-bit bus mode. 
#define FUNCTION_SET_INIT_1		0b00110000  	// - 8-bit bus mode.
#define FUNCTION_SET_INIT_2		0b00110000  	// - 8-bit bus mode.
#define FUNCTION_SET_INIT_4bit		0b00100000   	// - 4-bit bus mode.
#define INSTRCT_SET_0    		0b00101001      
#define INSTRCT_SET_1 			0b00101000      
#define BIAS_SET			0b00011101      // - Bias selection 1/4.
#define POWER_CONTROL 			0b01010000      // - Power control.
#define FOLLOWER_CONTROL		0b01101100      // - Internal follower curcuit ON.
#define CONTRAST_SET			0b01111000      // - Adjustment contrast.
#define DISP_ON				0b00001111      // - Entire display ON / Cursor ON / Cursor posistion ON.
#define ENTRY_MODE 			0b00000110      // - Cursor direction and display shift.


void enable()
{							
	LCD_PORT &= ~(1 << PIN_ENABLE);	// - Setting Enable to 0.	
	LCD_PORT |=  (1 << PIN_ENABLE);	// - Setting Enable to 1.
}

// - Writing 8-bit to LCD in two 'parts' (2 x 4-bit).
void LCD_FULL(unsigned char RS, unsigned char RW, unsigned char data)
{	
	// - Writing the high nibble first
	LCD_PORT  = (1 << PIN_ENABLE);				
	LCD_PORT |= (data & 0xF0) | (RS << PIN_RS) | (RW << PIN_RW);  // - Masking and obtaining the 4 MSB.
	
	// - Toggling Enable-bit to send data.
	enable();																	
	
	// - Writing the low nibble.
	LCD_PORT  = (1 << PIN_ENABLE);												
	LCD_PORT |= (data << 4) | (RS << PIN_RS) | (RW << PIN_RW);    // - Masking & left-shifting to obtain the 4 LSB.
	enable();
}


// - 4-bit.
void LCD_HALF(unsigned char RS, unsigned char RW, unsigned char data)
{
	LCD_PORT  = (1 << PIN_ENABLE);		
	LCD_PORT |= (data & 0xF0) | (RS << PIN_RS) | (RW << PIN_RW);
	
	// - Toggling Enable-bit to send data.
	enable();
}

// Determines which row & column the text is placed.
void set_cursor(char row, char column) 
{
	LCD_FULL(0, 0, (0b10000000 + row*16 + column) );		// - Char buffer-base address + (row * chars per row) + no. cols
}


void lcd_string(unsigned char row, unsigned char column, char *string) 
{
	set_cursor(row, column);		// - set row and column for text
	while(*string)
  {									
      LCD_FULL(1, 0, *string++);
      _delay_us(28);
	}
}

void LCD_clear () 
{
	LCD_FULL(0,0,CLEAR_DISP); _delay_ms(2);		// - Send 'CLEAR DISPLAY' to LCD_FULL.
}


void LCD_print_numb (unsigned char row, unsigned char column, unsigned int num) 
{
    char send_array[4];				
    sprintf(send_array, "%4d", num);			
		set_cursor(row, column);														
    for(unsigned char i = 0; i < 4; i++)
    {			
        LCD_FULL(1, 0, send_array[i]);	 
    }
}


void initialize_display() 
{	
	_delay_ms(40);	 // - Waiting 40 ms to ensure stabilized voltage.
	
	LCD_HALF(0, 0, FUNCTION_SET_INIT_0);	_delay_ms(2); 	// - Set 8-bit mode.
	LCD_HALF(0, 0, FUNCTION_SET_INIT_1);	_delay_ms(2);  	// - Set 8-bit mode.
	LCD_HALF(0, 0, FUNCTION_SET_INIT_2);	_delay_ms(2);   // - Set 8-bit mode.
	LCD_HALF(0, 0, FUNCTION_SET_INIT_4bit);	_delay_ms(2);   // - Set 4-bit mode.
		
	// - Activate the rest
	LCD_FULL(0, 0, INSTRCT_SET_0);	_delay_us(28);   // - Function set 0.
	LCD_FULL(0, 0, BIAS_SET);		_delay_us(28);   // - Bias set.
	LCD_FULL(0, 0, POWER_CONTROL);	_delay_us(28);   // - Power control.
	LCD_FULL(0, 0, FOLLOWER_CONTROL);	_delay_us(28);   // - Follower control.
	LCD_FULL(0, 0, CONTRAST_SET);	_delay_us(28); 	 // - Contrast set.
	LCD_FULL(0, 0, INSTRCT_SET_1);	_delay_us(28);   // - Function set 1.
	LCD_FULL(0, 0, DISP_ON);		_delay_us(28);   // - Display ON.
	LCD_FULL(0, 0, CLEAR_DISP);	_delay_ms(3);    // - Clear display.
	LCD_FULL(0, 0, ENTRY_MODE);	_delay_us(28);   // - Entry mode set.
}
