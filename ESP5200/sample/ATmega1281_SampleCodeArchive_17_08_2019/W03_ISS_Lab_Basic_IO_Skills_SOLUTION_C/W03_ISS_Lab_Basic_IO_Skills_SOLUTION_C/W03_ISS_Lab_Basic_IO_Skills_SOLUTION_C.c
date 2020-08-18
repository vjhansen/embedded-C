/*******************************************
Project 		W03 Lab - Basic IO Skills SOLUTION (C version)
Target 			ATmega1281 on STK300
Program			W03_ISS_Lab_Basic_IO_Skills_SOLUTION_C.c
Author			Richard Anthony
Date			2nd October 2013

Function		Counts switch presses, displays value on LEDs
*******************************************/

#include <avr/io.h>
#include <util/delay.h>				// Add this header so the _delay_ms() function can be used

void InitialiseGeneral();

unsigned char SwitchesValue;		// variable to remember the switches value so we only have to use PIND once in the loop
unsigned char SwitchPressCount;		// variable to remember count of number of switch presses
unsigned char CountHasReached10;	// variable that remembers if the 'reached 10' display has been done

void InitialiseGeneral()
{
	DDRB = 0xFF;	// Set port B direction OUTPUT (connected to the on-board LEDs)
	PORTB = 0xFF;	// Set all LEDs initially off (inverted on the board, so '1' = off)

	DDRD = 0x00;	// Set port D direction INPUT (connected to the on-board SWITCHs)

	// Initialise variables
	CountHasReached10 = 0;
	SwitchPressCount = 0;
}

int main( void )
{
	InitialiseGeneral();

	while(1)
	{
		// The core of the answer (part 2 of the task)
		SwitchesValue = PIND;				// Read the value of the switches
		// If using the external switches and lights box, you do not need to invert the bits but
		// if using the on-board switches you do need to
		SwitchesValue = ~ SwitchesValue;	// Invert the bits (when using the on-board switches)
		if(0b00000001 == SwitchesValue)		// Was switch 0 pressed ?
		{
			SwitchPressCount++;				// If code gets here, the switch was pressed, so add 1 to the count
		}


		// The statement below is part of the solution to part 3
		Display_Count_On_LEDs();


		// The statement below is part of the solution to part 2A
		DELAY_debounce();


		// The statements below are part of the solution to part 4
		if(SwitchPressCount >= 0b00010100)	// Compare the count with the value 20
		{
			Reset_Count_And_Update_Display();
		}


		// The statements below are part of the solution to part 5
		if(0b00001010 == SwitchPressCount)		// Compare the count with the value 10
		{
			if(0x00 == CountHasReached10)		// Test whether this code has been executed already for the current count sequence
												// (otherwise the part 5 code runs every time round the loop until the count increases to 11)
			{
				CountHasReached10 = 0x01;		// Set the variable so this section of code only runs the first time the count value is detected as being 10
				PORTB = 0x00;					// Turn all LEDs on
				DELAY_1Second();				// Leave LEDs on for 1 second
				Display_Count_On_LEDs();		// Display the count value again
			}
		}

		// The statements below are part of the solution to part 6
		if(0b10000000 == SwitchesValue)			// Was switch 7 pressed ?
		{
			Reset_Count_And_Update_Display();	// If code gets here, switch 7 was pressed, so reset the count and update the display
		}
	}
}


// This is the solution to part 3 of the task
void Display_Count_On_LEDs()
{
	PORTB = ~ SwitchPressCount; // Invert (1's complement)and display the value on the LEDs
}

// This code is added as part of the solution to part 4 and 6 of the task
void Reset_Count_And_Update_Display()
{
	SwitchPressCount = 0;		// Reset the count to 0
	Display_Count_On_LEDs();
	CountHasReached10 = 0;		// Reset the variable that remembers if the 'reached 10' display has been done
}

// Short debounce delay routine needed in part 2A of the task
void DELAY_debounce() // 0.2 second
{
	_delay_ms(200);
}

// 1 second delay routine needed in part 5 of the task
void DELAY_1Second()
{
	_delay_ms(1000);
}
