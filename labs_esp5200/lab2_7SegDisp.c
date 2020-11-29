#include <avr/io.h>
#include <util/delay.h>

#define F_CPU 1000000UL //Assumes 1MHz clock speed

void InitialiseGeneral();
void CountUpDown();
void Seg7();
unsigned char uCountvalue;	// Create the variable that will hold the count value

void InitialiseGeneral()
{
	// Configure Ports
	DDRA = 0xFF;	// Set port A direction OUTPUT (connected to the on-board LEDs)
	PORTA = 0xFF;	// Set all LEDs initially off (inverted on the board, so '1' = off)
	
	DDRC = 0x00;	// Port C as INPUT
	PORTC = 0xFF;	// Port C off (inverted on the board, so '1' = off)

	uCountvalue  = 0;	// Initialize the count value to 0
}

int main( void )
{
	InitialiseGeneral();

	while(1)
	{
		//CountUpDown();
		Seg7();
	}
}

// ---------------------------------------------------
// PART 2 - Count up or down / Stop Start with pin

void CountUpDown()
{
	if (PINC & 0b01000000) // Start/Stop	(Arduino Pin 31)
	{
		if(PINC & 0b10000000) // Up/down	(Arduino Pin 30)
		{
			PORTA = ~uCountvalue;
			uCountvalue--;
		}
		
		else
		{
			PORTA = ~uCountvalue;
			uCountvalue++;
		}
	} 
	
	else
	{
		PORTA = ~uCountvalue;
	}
	
	_delay_ms(500);			// Comment for debugging, Add a delay so we can see the pattern change
}


// ---------------------------------------------------
// PART 3 - 7 Segment display

/*  
				7    G - PORTA7		(Arduino Pin 29)
// --A--		6    B - PORTA6		(Arduino Pin 28)
// F    B		5    D - PORTA5		(Arduino Pin 27)
// --G--		4    F - PORTA4		(Arduino Pin 26)
// E    C		3    DP- PORTA3		(Arduino Pin 25)
// --D-- (DP)	2    E - PORTA2		(Arduino Pin 24)
				1    A - PORTA1		(Arduino Pin 23)
				0    C - PORTA0		(Arduino Pin 22)
*/
void Seg7(){      
    // 0 = led on
	switch (uCountvalue) {
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
        default:
            break;
    }
    uCountvalue++;
    if (uCountvalue > 9) {
        uCountvalue = 0;
    }

    _delay_ms(500);         // Add a delay so we can see the pattern change
    }
