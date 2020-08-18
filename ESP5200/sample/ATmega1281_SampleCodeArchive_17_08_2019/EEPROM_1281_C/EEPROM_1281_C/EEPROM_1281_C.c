//*******************************************************************************
// Project 		EEPROM sample application (Embedded C)
// Target 		ATMEL ATmeg1281 micro-controller on Header board
// Program		EEPROM_1281_C.c
// Author		Richard Anthony
// Date			15th October 2015, (original ATmega8535 version 19th October 2012)

// Function		Store and display a power-on-count value on the LEDs connected to PortB
//				Specifically:
//					Each time the board is powered on the previous PowerOnCount value is retrieved from EEPROM (demonstrating the non-volatile nature of the EEPROM storage)
//					The PowerOnCount is incremented
//					The new PowerOnCount value is written back to the EEPROM
//					The new PowerOnCount value is displayed on the LEDs for diagnostic purposes
//*******************************************************************************

#include <avr/io.h>
#include "EEPROM_Functions_1281.h"

#define PowerOnCount_AddressInEEPROM 0	// The address in EEPROM where the PowerOnCount value will be stored

// Function declarations
void InitialiseGeneral();

int main(void)
{
	unsigned char ucPowerOnCount;	// Define the variable as an 8-bit value
	InitialiseGeneral();

	ucPowerOnCount = EEPROM_read_Byte(PowerOnCount_AddressInEEPROM);	// Read the current value
	ucPowerOnCount++;													// Increment the value
	EEPROM_write_Byte(PowerOnCount_AddressInEEPROM, ucPowerOnCount);	// Write the updated value into EEPROM
	PORTB = ~ucPowerOnCount;											// Display the new value on the LEDs

    while(1)
    {
    }
}

void InitialiseGeneral()
{
	DDRB = 0xFF;		// Configure PortB direction for Output
	PORTB = 0xFF;		// Set all LEDs initially off (inverted on the board, so '1' = off)
}
