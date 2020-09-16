//For 'DOG 3 x 16'
//Definerer hvilke registre og pinner som hører til LCD-pinnene CSB, CLK, SI og RS:

#define lcd_csb_port PORTD
#define lcd_csb_ddr  DDRD
#define lcd_csb_pin 4

#define lcd_clk_port PORTD
#define lcd_clk_ddr  DDRD
#define lcd_clk_pin 5

#define lcd_si_port PORTD
#define lcd_si_ddr  DDRD
#define lcd_si_pin  6

#define lcd_rs_port PORTC
#define lcd_rs_ddr  DDRC
#define lcd_rs_pin 4

#define F_CPU 14745600
#include <util/delay.h>

//Enable-signal ------------------------------------------------------------------------------
void strobe(void) 
{
	//_delay_us(1);          // for fast clk! Progget for 14.745Mhz
	lcd_clk_port &= ~(1 << lcd_clk_pin);
	//_delay_us(1);
	lcd_clk_port |= (1 << lcd_clk_pin);
}

// Set instruksjoner ut i en SPI-struktur ---------------------------------------------------
void lcdinst(unsigned char txb, unsigned char RS)
{
	lcd_csb_port &= ~(1 << lcd_csb_pin);
	lcd_rs_port &= ~(1 << lcd_rs_pin);
	lcd_rs_port |= (RS << lcd_rs_pin);
	
	if (txb&128) lcd_si_port |= (1<<lcd_si_pin); 
		else lcd_si_port &= ~(1<<lcd_si_pin);  strobe();
	if (txb&64)  lcd_si_port |= (1<<lcd_si_pin); 
		else lcd_si_port &= ~(1<<lcd_si_pin);  strobe();
	if (txb&32) lcd_si_port  |= (1<<lcd_si_pin); 
		else lcd_si_port &= ~(1<<lcd_si_pin);  strobe();
	if (txb&16) lcd_si_port  |= (1<<lcd_si_pin); 
		else lcd_si_port &= ~(1 << lcd_si_pin);  strobe();
	if (txb&8) lcd_si_port   |= (1 << lcd_si_pin); 
		else lcd_si_port &= ~(1 << lcd_si_pin);  strobe();
	if (txb&4) lcd_si_port   |= (1 << lcd_si_pin); 
		else lcd_si_port &= ~(1 << lcd_si_pin);  strobe();
	if (txb&2) lcd_si_port   |= (1 << lcd_si_pin); 
		else lcd_si_port &= ~(1 << lcd_si_pin);  strobe();
	if (txb&1) lcd_si_port   |= (1 << lcd_si_pin); 
		else lcd_si_port &= ~(1 << lcd_si_pin);  strobe();
	
	lcd_csb_port|=(1<<lcd_csb_pin);
}

// set cg-ram-adress - define own egne chars
void cgramaddressset(unsigned char cgaddress, unsigned char line) 
{
	lcdinst(cgaddress*8+line+64,0);    _delay_us(40);
}

// set cursor in display
void lcd_cursoron(void) 
{
	lcdinst(0b00001110, 0);    _delay_us(40);
}

// cursor off
void lcd_cursoroff(void) 
{
	lcdinst(0b00001100, 0);    _delay_us(40);
}

// define cg-ram
void init_cgram(void)
{
	int cgaddress,line;
	for (cgaddress=0; cgaddress < 8; cgaddress=cgaddress+1) {
		for (line=0; line<8; line=line+1) {
			cgramaddressset(cgaddress, line);
			switch(cgaddress){
				case 0:
				switch(line){       //æ
					case 0: lcdinst(0b00000000,1);    _delay_us(40);      break;
					case 1: lcdinst(0b00000000,1);    _delay_us(40);      break;
					case 2: lcdinst(0b00001110,1);    _delay_us(40);      break;
					case 3: lcdinst(0b00010101,1);    _delay_us(40);      break;
					case 4: lcdinst(0b00001111,1);    _delay_us(40);      break;
					case 5: lcdinst(0b00010100,1);    _delay_us(40);      break;
					case 6: lcdinst(0b00001111,1);    _delay_us(40);      break;
					case 7: lcdinst(0b00000000,1);    _delay_us(40);      break;
					default: break;
				};
				case 1:
				switch(line){       //ø
					case 0: lcdinst(0b00000000,1);    _delay_us(40);      break;
					case 1: lcdinst(0b00000000,1);    _delay_us(40);      break;
					case 2: lcdinst(0b00001110,1);    _delay_us(40);      break;
					case 3: lcdinst(0b00010011,1);    _delay_us(40);      break;
					case 4: lcdinst(0b00010101,1);    _delay_us(40);      break;
					case 5: lcdinst(0b00011001,1);    _delay_us(40);      break;
					case 6: lcdinst(0b00001110,1);    _delay_us(40);      break;
					case 7: lcdinst(0b00000000,1);    _delay_us(40);      break;
					default: break;
				};
				case 2:
				switch(line){       //å
					case 0: lcdinst(0b00000100,1);    _delay_us(40);      break;
					case 1: lcdinst(0b00000000,1);    _delay_us(40);      break;
					case 2: lcdinst(0b00001110,1);    _delay_us(40);      break;
					case 3: lcdinst(0b00000001,1);    _delay_us(40);      break;
					case 4: lcdinst(0b00001111,1);    _delay_us(40);      break;
					case 5: lcdinst(0b00010001,1);    _delay_us(40);      break;
					case 6: lcdinst(0b00001111,1);    _delay_us(40);      break;
					case 7: lcdinst(0b00000000,1);    _delay_us(40);      break;
					default:  break;
				};
				case 3:
				switch(line){       //Æ
					case 0: lcdinst(0b00000111,1);    _delay_us(40);      break;
					case 1: lcdinst(0b00001100,1);    _delay_us(40);      break;
					case 2: lcdinst(0b00010100,1);    _delay_us(40);      break;
					case 3: lcdinst(0b00011111,1);    _delay_us(40);      break;
					case 4: lcdinst(0b00010100,1);    _delay_us(40);      break;
					case 5: lcdinst(0b00010100,1);    _delay_us(40);      break;
					case 6: lcdinst(0b00010111,1);    _delay_us(40);      break;
					case 7: lcdinst(0b00000000,1);    _delay_us(40);      break;
					default:  break;
				};
				
				case 4:
				switch(line){       //Å
					case 0: lcdinst(0b00000100,1);    _delay_us(40);      break;
					case 1: lcdinst(0b00000000,1);    _delay_us(40);      break;
					case 2: lcdinst(0b00000100,1);    _delay_us(40);      break;
					case 3: lcdinst(0b00001010,1);    _delay_us(40);      break;
					case 4: lcdinst(0b00001110,1);    _delay_us(40);      break;
					case 5: lcdinst(0b00010001,1);    _delay_us(40);      break;
					case 6: lcdinst(0b00010001,1);    _delay_us(40);      break;
					case 7: lcdinst(0b00000000,1);    _delay_us(40);      break;
					default: break;
				};
				case 5:
				switch(line){       // :(
					case 0: lcdinst(0b00000000,1);    _delay_us(40);      break;
					case 1: lcdinst(0b00001010,1);    _delay_us(40);      break;
					case 2: lcdinst(0b00001010,1);    _delay_us(40);      break;
					case 3: lcdinst(0b00000000,1);    _delay_us(40);      break;
					case 4: lcdinst(0b00010001,1);    _delay_us(40);      break;
					case 5: lcdinst(0b00010001,1);    _delay_us(40);      break;
					case 6: lcdinst(0b00001110,1);    _delay_us(40);      break;
					case 7: lcdinst(0b00000000,1);    _delay_us(40);      break;
					default: break;
				};
				case 6:
				switch(line){       // :)
					case 0: lcdinst(0b00000000,1);    _delay_us(40);      break;
					case 1: lcdinst(0b00001010,1);    _delay_us(40);      break;
					case 2: lcdinst(0b00001010,1);    _delay_us(40);      break;
					case 3: lcdinst(0b00000000,1);    _delay_us(40);      break;
					case 4: lcdinst(0b00001110,1);    _delay_us(40);      break;
					case 5: lcdinst(0b00010001,1);    _delay_us(40);      break;
					case 6: lcdinst(0b00010001,1);    _delay_us(40);      break;
					case 7: lcdinst(0b00000000,1);    _delay_us(40);      break;
					default: break;
				};
				case 7:
				switch(line){       // lighting
					case 0: lcdinst(0b00000001,1);    _delay_us(40);      break;
					case 1: lcdinst(0b00000010,1);    _delay_us(40);      break;
					case 2: lcdinst(0b00000100,1);    _delay_us(40);      break;
					case 3: lcdinst(0b00000010,1);    _delay_us(40);      break;
					case 4: lcdinst(0b00010100,1);    _delay_us(40);      break;
					case 5: lcdinst(0b00011000,1);    _delay_us(40);      break;
					case 6: lcdinst(0b00011100,1);    _delay_us(40);      break;
					case 7: lcdinst(0b00000000,1);    _delay_us(40);      break;
					default: break;
				};
				default: break;
			};
		};
	};
}

// init lcd, setter rette pinner ut osv...
void init_lcd(void)
{
	//_delay_ms(20);
	lcd_rs_ddr  |= (1 << lcd_rs_pin);
	lcd_clk_ddr |= (1 << lcd_clk_pin);
	lcd_si_ddr  |= (1 << lcd_si_pin);
	lcd_csb_ddr |= (1 << lcd_csb_pin);
	
	lcd_rs_port  |= (1 << lcd_rs_pin);
	lcd_clk_port |= (1 << lcd_clk_pin);
	lcd_si_port. |= (1 << lcd_si_pin);
	lcd_csb_port |= (1 << lcd_csb_pin);
	_delay_ms(40);

	lcdinst(0b00111001,0); _delay_us(1100);
	lcdinst(0b00011101,0); _delay_us(30);
	lcdinst(0b01010010,0); _delay_us(30);
	lcdinst(0b01101001,0); _delay_us(30);
	lcdinst(0b01110100,0); _delay_us(30);
	lcdinst(0b00111000,0); _delay_us(30);
	lcdinst(0b00001100,0); _delay_us(30);
	lcdinst(0b00000001,0); _delay_us(1100);
	lcdinst(0b00000110,0); _delay_us(30);

	init_cgram();
}

// setter cursoren på rett sted, row kan være 0, 1 eller 2, col 0->15
void dotpos(int row, int col) //sets marker ready to write on row,col
{  
	unsigned char instr=0;
	switch(row){
		case 0: instr=0x80; break;
		case 1: instr=0x90; break;
		case 2: instr=0xA0; break;
	default: break;}
	lcdinst(instr+col%16,0);  _delay_us(39);
}

//funksjon som skriver ut en tekst på plassen [row,col]
void lcd_printline (char row, char col, char *a)
{
	dotpos(row,col);
	while(*a) {
		if     (*a=='Æ') lcdinst(0b10010010,1);
		else if(*a=='æ') lcdinst(0b10010001,1);
		else if(*a=='Ø') lcdinst(0b11101110,1);
		else if(*a=='ø') lcdinst(0b11101111,1);
		else if(*a=='Å') lcdinst(0b10001111,1);
		else if(*a=='å') lcdinst(0b10000110,1);
		else
		lcdinst(*a,1);
		_delay_us(43);
		*a++;
	}
}

//funksjon for å skrive ut en og en bokstav
void lcd_printchar (unsigned char a) 
{
	if(a=='Æ')	lcdinst(0b10010010,1);
	else if(a=='æ') lcdinst(0b10010001,1);
	else if(a=='Ø') lcdinst(0b11101110,1);
	else if(a=='ø') lcdinst(0b11101111,1);
	else if(a=='Å') lcdinst(0b10001111,1);
	else if(a=='å') lcdinst(0b10000110,1);
	else lcdinst(a,1);
	_delay_us(43);
}

//funksjon for å skrive ut tall på plass [row, col]. Tallet har d siffer, og tallet er nb
void lcd_printnb(char row, char col, char d, long int nb)
{
	char tobeprinted;
	char i;
	long int fact;
	dotpos(row,col);
	while(d > 0) {
		fact=1;
		for(i=1; i<d; i++)fact=fact*10;
		tobeprinted=nb/fact;
		lcdinst(tobeprinted+48, 1); _delay_us(43);
		nb=nb-(tobeprinted*fact);
		d--;
	}
}
