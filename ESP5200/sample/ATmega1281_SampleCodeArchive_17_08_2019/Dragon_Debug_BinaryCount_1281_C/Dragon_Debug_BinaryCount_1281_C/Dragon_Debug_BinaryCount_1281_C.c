/*******************************************
Project 		Binary Count On LEDs (version with Dragon / Debug guidance comments added)
Target 			ATmega1281 on STK300
Program			Dragon_Debug_BinaryCount_1281_C.c
Author			Richard Anthony
Date			14th September 2013

Fuse settings	System clock: Use the internal RC oscillator at 8.0MHz and CKDIV8 fuse programmed, resulting in 1.0MHz system clock.
				Fuse settings should be: CKSEL = "0010", SUT = "10", CKDIV8 = "0"

Function		Counts in binary and displays output on the on-board LEDs
				Contains guidance comments for debugging with the Dragon / JTAG device
*******************************************/

/* The DRAGON board uses the JTAG interface to the ATmega1281 to provide a programming and debug facility

Activity sequence is:
	1. Develop your program as usual, using ATMEL Studio 6.
	(create a project, type your code, build the code and remove any errors that are detected)

	2. Connect the hardware
	(the Dragon board connects to the PC via USB, and to the STK300 board via an IDC10 cable, 
	the Dragon board is powered through the USB, but the STK300 needs an external power supply)

	3. Program the ATmega1281
	   Under the 'Tools' tab in Atmel Studio 6, select Device Programming
	3A	Make the following selections in the configuration window:
			Tool = AVR Dragon
			Device = ATmega1281
			Interface = JTAG

	3B	click 'Apply'

	3C	Check that the Dragon board is correctly interfaced to the ATmega1281 - this is done by clicking the 'Read' button
		in the 'Device Signature' box. 
		A device specific hex number should be shown, also the operating voltage will also be displayed (this will depend 
		on how you have configured the board but the default value should be 5Volts, or close to this value).
		As long as a signature value is displayed and the voltage is as expected, continue - otherwise fault-find this step before progressing.

	3D	Click 'Memories' from the left-hand pane. This opens the memory-programming pane on the right.
		For Flash memory, select the .hex file that you wish to load - this requires browsing to your project directory and the debug directory within it, where you will find the .hex file.
		Ensure that the Erase and Verify tick boxes are selected.
		
	3E	Click the 'Program' button - this actually loads the hex code file into the Atmel Flash memory.
		At the bottom left of the Device Programming window you should see:
			Erasing device... OK
			Programming Flash...OK
			Verifying Flash...OK
		This confirms that the program has been loaded correctly
		
	4	Single-step through the program code
	    This step is where we use the debug facility of the Dragon board to see how our program works one instruction at a time
		
	4A	Add a breakpoint on the statement that outputs the data value to PORT B
	    This is done by placing the cursor on the statement and pressing F9

	4B Under the 'Debug' tab in Atmel Studio 6, select Start Debugging and Break
	   The 'Select Tool' window will now appear. Within this window, select the Dragon device (blue highlight) and press 'OK'.

	4C You are now in debug mode. The first instruction of the program will be highlighted yellow, and an arrow will point at the next instruction to be executed.
	   The following function keys perform the debugging functions:
			F5				run to next breakpoint
			F9				add / remove a breakpoint (toggles)
			F10				step over a block or call
			F11				step into a block or follow a call into lower level
			Shift + F11		step out to previous (higher) level
			Cntl + F10		run to cursor
			
	4D	Experiment with these commands until you are confident with the debugging facility
	
		Note that you can hover the mouse over the 'uCountvalue' variable to see its value as the program runs, 
		see that its value increases by 1 each time the code goes around the loop
	
		Also, you can copy the 'uCountvalue' variable into a 'Watch' window at the bottom of the screen and see 
		the value changing dynamically as the program runs - experiment with this.
	
		Note also that the LEDs value will actually change as you step through the code because the program is actually running
		- i.e. this is not a simulation
*/

#include <avr/io.h>
#include <util/delay.h>

void InitialiseGeneral();
unsigned char uCountvalue;	// Create the variable that will hold the count value

void InitialiseGeneral()
{
	// Configure Ports
	DDRB = 0xFF;	// Set port B direction OUTPUT (connected to the on-board LEDs)
	PORTB = 0xFF;	// Set all LEDs initially off (inverted on the board, so '1' = off)

	uCountvalue  = 0;	// Initialize the count value to 0
}

int main( void )
{
	InitialiseGeneral();

	while(1)
	{
		// Use F9 to place a breakpoint on the line below
		PORTB = ~uCountvalue;	// Write the data value to Port B (the ~ performs 1s compliment
								// because the switch values are inverted in the STK300 board hardware)
	
		uCountvalue++;			// Increment the count value

		// The delay statement below has been commented out because it complicates the debugging process - especially for beginners
		// Note that if running the code without debug mode you should un-comment the statement otherwise the display changes too quickly to see what is happening 
		//_delay_ms(500);			// Add a delay so we can see the pattern change
	}
}
