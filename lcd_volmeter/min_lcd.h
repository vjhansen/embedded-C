/*
Library for LCD EA DOGM163 (4-bit/5V) 16x2.

uC: ATmega16.
IDP: Atmel Studio 7.

*/


#define LCD_PORT	PORTC   // - PORTC acts as our LCD PORT.
#define PIN_RW		PC0	// - PINC.0 is Read/write.
#define PIN_RS 		PC1	// - PINC.1 is Registry selector.
#define PIN_ENABLE 	PC2	// - PINC.2 is Enable.

// - Defining instructions for the LCD.
#define CLEAR_DISP    			0b00000001	// - Clear display.
#define FUNCTION_SET_INIT_0		0b00110000  	// - 8-bit bus mode. 
#define FUNCTION_SET_INIT_1		0b00110000  	// - 8-bit bus mode.
#define FUNCTION_SET_INIT_2		0b00110000  	// - 8-bit bus mode.
#define FUNCTION_SET_INIT_4bit		0b00100000   	// - 4-bit bus mode.
#define INSTRCT_SET_0    		0b00101001      // - Function instruction set 0.
#define INSTRCT_SET_1 			0b00101000      // - Function instruction set 1.
#define BIAS_SET			0b00011101      // - Bias selection 1/4.
#define POWER_CONTROL 			0b01010000      // - Power control.
#define FOLLOWER_CONTROL		0b01101100      // - Internal follower curcuit ON.
#define CONTRAST_SET			0b01111000      // - Adjustment contrast.
#define DISP_ON				0b00001111      // - Entire display ON / Cursor ON / Cursor posistion ON.
#define ENTRY_MODE 			0b00000110      // - Setter retningen p� cursor og display shift.

// - Setter opp kommunikasjon med displayet.
void enable(){							
	LCD_PORT &=~ (1<<PIN_ENABLE);	// - Setting Enable to 0.
	LCD_PORT |=  (1<<PIN_ENABLE);	// - Setting Enable to 1.
}

// - Writing 8-bit to LCD in two 'parts' (2 x 4-bit).
void LCD_FULL(unsigned char RS, unsigned char RW, unsigned char data){
	
	// - Writing the high nibble first
	LCD_PORT  = (1<<PIN_ENABLE);						
	LCD_PORT |= (data&0xF0)|(RS<<PIN_RS)|(RW<<PIN_RW);  // - Masking and obtaining the 4 MSB.
	
	// - Toggling Enable-bit to send data.
	enable();																	
	
	// - Writing the low nibble.
	LCD_PORT  = (1<<PIN_ENABLE);												
	LCD_PORT |= (data<<4)|(RS<<PIN_RS)|(RW<<PIN_RW);    // - Masking & left-shifting to obtain the 4 LSB.
	enable();
}

// - 4-bit.
void LCD_HALF(unsigned char RS, unsigned char RW, unsigned char data){
	LCD_PORT  = (1<<PIN_ENABLE);		
	LCD_PORT |= (data&0xF0)|(RS<<PIN_RS)|(RW<<PIN_RW);	// - Henter de 4 MSB
	
	// - Toggling Enable-bit to send data.
	enable();
}

// - This function determines which row & column the text is placed.
void set_cursor(char row, char column) {
	LCD_FULL(0,0,((0b10000000 + row*16 + column)));		// - Char buffer-base adresse + (rad * chars per rad) + antall kolonner.
}

// - Print string function.
void lcd_string(unsigned char row, unsigned char column, char *string) {
	set_cursor(row, column);		// - set row and column for text
	while(*string) {			// - loop through string									
		LCD_FULL(1,0,*string++);	// - write to LCD with 'LCD_FULL'.
		_delay_us(28);
	}
}

// - Clear display.
void LCD_clear () {
	LCD_FULL(0,0,CLEAR_DISP); _delay_ms(2);		// - Send 'CLEAR DISPLAY' to LCD_FULL.
}

// - Print number function.
void LCD_print_numb (unsigned char row, unsigned char column, unsigned int tall) {
	char send_array[4];					// - Setter av et array som holder 4 chars.
	sprintf(send_array,"%4d", tall);			// - Gir sprinft 4 digits..
		set_cursor(row, column);			// - ..og angir hvor de skal plasseres.													
	for(unsigned char i = 0; i < 4; i++) {			// - For-loop som itererer gjennom 'send_array' 4 ganger.
		LCD_FULL(1,0,send_array[i]);			// - Sender hvert enkelt tall fra arrayet til LCD_FULL.
	}
}

// - This function initializes the display by executing our defined instructions.
void initialize_display() {
	
	_delay_ms(40);	 // - Waiting 40 ms to ensure stabilized voltage.
	
	LCD_HALF(0,0,FUNCTION_SET_INIT_0);	_delay_ms(2); 	// - Set 8-bit mode.
	LCD_HALF(0,0,FUNCTION_SET_INIT_1);	_delay_ms(2);  	// - Set 8-bit mode.
	LCD_HALF(0,0,FUNCTION_SET_INIT_2);	_delay_ms(2);   // - Set 8-bit mode.
	LCD_HALF(0,0,FUNCTION_SET_INIT_4bit);	_delay_ms(2);   // - Set 4-bit mode.
		
	// - Aktiverer alt annet.
	LCD_FULL(0,0,INSTRCT_SET_0);	_delay_us(28);   // - Function set 0.
	LCD_FULL(0,0,BIAS_SET);		_delay_us(28);   // - Bias set.
	LCD_FULL(0,0,POWER_CONTROL);	_delay_us(28);   // - Power control.
	LCD_FULL(0,0,FOLLOWER_CONTROL);	_delay_us(28);   // - Follower control.
	LCD_FULL(0,0,CONTRAST_SET);	_delay_us(28); 	 // - Contrast set.
	LCD_FULL(0,0,INSTRCT_SET_1);	_delay_us(28);   // - Function set 1.
	LCD_FULL(0,0,DISP_ON);		_delay_us(28);   // - Display ON.
	LCD_FULL(0,0,CLEAR_DISP);	_delay_ms(3);    // - Clear display.
	LCD_FULL(0,0,ENTRY_MODE);	_delay_us(28);   // - Entry mode set.
}
