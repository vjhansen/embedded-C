//*******************************************************************************
// Project 		EEPROM library functions (Embedded C)
// Target 		ATMEL ATmeg1281 micro-controller on Header board
// Program		EEPROM_Functions_1281.c
// Author		Richard Anthony
// Date			15th October 2015, (original ATmega8535 version 19th October 2012)

// Notes		EEPROM is initialised to 0xFF's
//*****************************************

// Microcontroller-specific register addresses
// ATmega1281
// 0x1F is EECR register
// 0x22 is EEARH register
// 0x21 is EEARL register
// 0x20 is EEDR register

#include "EEPROM_Functions_1281.h"

void EEPROM_write_Byte(unsigned int uiAddress, unsigned char ucData)
{		
	// Assembly routine needed here as the access to EEPROM is timing critical and 'write' DOES NOT work in C
	unsigned char cAddress_H = uiAddress / 256;		// The upper byte of address value
	unsigned char cAddress_L = uiAddress % 256;		// The lower byte of address value

asm volatile (
	"CLI\n\t"						// Disable global interrupts because of the timing critical nature of this code
	
	"EEPROM_write1_ASM:\n\t"		// EEPROM_write_ASM:	label for looping back to while waiting for completion of previous write
	"sbic 0x1F,1\n\t"				// 0x1F is EECR register, EEPE is bit 1 in the EECR register
	"rjmp EEPROM_write1_ASM\n\t"	// Wait until previous write has completed
	
									// Set the EEPROM address to write to
	"out 0x22, %0\n\t"				// 0x22 is EEARH register, %0 maps onto the upper byte of address in the input parameter cAddress_H
	"out 0x21, %1\n\t"				// 0x21 is EEARL register, %1 maps onto the lower byte of address in the input parameter cAddress_L
									
									// Set the data value to write
	"out 0x20,%2\n\t"				// 0x20 is EEDR register, %2 maps onto the input parameter ucData
	
									// Write logical one to EEMPE
	"sbi 0x1F,2\n\t"				// 0x1F is EECR register, EEMPE is bit 2 in the EECR register
	
									// Start EEPROM write by setting EEPE
	"sbi 0x1F,1\n\t"				// 0x1F is EECR register, EEPE is bit 1 in the EECR register
	
	"SEI"							// Re-enable global interrupts
	
	:													// Output parameters (none in this case)
	: "a"(cAddress_H), "a"(cAddress_L), "a" (ucData)	// Input parameters in the order mapped onto %0 %1 %2
	:													// Affected general-purpose registers (none in this case), example syntax:	"%r16","%r17","%r18" 
	 );
}

unsigned char EEPROM_read_Byte(unsigned int uiAddress)
{
	// Assembly routine needed here as the access to EEPROM is timing critical and 'read' MAY not work reliably in C
	unsigned char cAddress_H = uiAddress / 256;		// The upper byte of address value
	unsigned char cAddress_L = uiAddress % 256;		// The lower byte of address value
	unsigned char ucData;

asm volatile (
	"CLI\n\t"						// Disable global interrupts because of the timing critical nature of this code
	
	"EEPROM_write2_ASM:\n\t"		// EEPROM_write_ASM:	label for looping back to while waiting for completion of previous write
	"sbic 0x1F,1\n\t"				// 0x1F is EECR register, EEPE is bit 1 in the EECR register
	"rjmp EEPROM_write2_ASM\n\t"	// Wait until previous write has completed
	
									// Set the EEPROM address to write to
	"out 0x22, %1\n\t"				// 0x22 is EEARH register, %0 maps onto the upper byte of address in the input parameter cAddress_H
	"out 0x21, %2\n\t"				// 0x21 is EEARL register, %1 maps onto the lower byte of address in the input parameter cAddress_L
									
									// Start EEPROM write by setting EEPE
	"sbi 0x1F,0\n\t"				// 0x1F is EECR register, EERE is bit 0 in the EECR register
	
									// Read the data value from the EEPROM
	"in %0,0x20\n\t"				// 0x20 is EEDR register, %0 maps onto the output parameter ucData
	
	"SEI"							// Re-enable global interrupts
	
	: "=r"(ucData)							// Output parameter (mapped onto %0)
	: "a"(cAddress_H), "a"(cAddress_L)	 	// Input parameters in the order mapped onto %1 %2
	:										// Affected general-purpose registers (none in this case), example syntax:	"%r16","%r17","%r18" 
	 );

	return ucData;
}
