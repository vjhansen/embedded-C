//*******************************************************************************
// Project 		EEPROM library functions (Embedded C)
// Target 		ATMEL ATmeg1281 micro-controller on Header board
// Program		EEPROM_Functions_1281.h
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

// Declarations
void EEPROM_write_Byte(unsigned int uiAddress, unsigned char ucData);
unsigned char EEPROM_read_Byte(unsigned int uiAddress);

