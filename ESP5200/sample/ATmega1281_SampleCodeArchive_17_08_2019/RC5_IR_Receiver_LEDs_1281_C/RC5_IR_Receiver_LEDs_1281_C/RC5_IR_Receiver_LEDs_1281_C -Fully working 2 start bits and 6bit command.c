//*****************************************
// Project 		IR receiver module - RC5 IR Remote Control Decoder (C)
// Target 		ATmega1281 on STK300
// Program		RC5_IR_Receiver_LEDs_1281_C.c
// Author		Richard Anthony
// Date			19th October 2013 (original ATmega8535 version 29th October 2011)

// Function		IR Receiver module, connected to Digital input pin

//	I/O			Reads IR module output via Port D bit 2 (H/W Int 2)
//				Outputs 'command' data to Port B

// The RC-5 protocol is described at: http://en.wikipedia.org/wiki/RC-5

//	IMPORTANT NOTE:
//			TESTED WITH NTL / VIRGIN MEDIA SET-TOP BOX REMOTE CONTROL - WORKS.
//			DOES NOT WORK WITH SOME TV REMOTES AS THEY DONT ALL USE THE STANDARD RC5 CODE

//***************************
// NOTE: Assumes IR module configured for bit 2, check this aspect if the code does not work correctly in the Lab
//***************************

//	System Clock	Timing calibrated for 1MHz System clock

//	RC5 code		A 14 bit code with the following structure:
//					Start (2 bits) to adjust the AGC level in the receiver IC
//					Control/toggle bit (1 bit)
//					System/device address (5 bits)	32 address values
//					Command (6 bits)				64 * 2 commands (see start-code values below)

//					Start bit values (first is always 1, second bit denotes which set of command codes)
//					10 = command codes 64-127
//					11 = command codes 0-63

//					Control bit value toggles after each key release (initiates a new transmission)

//	Symbol coding	Each symbol is 1.778ms long, split into two half-symbols, as follows:
//					High half-symbol followed by Low half-symbol = 0
//					Low half-symbol followed by High half-symbol = 1

//	Modulation		The HIGH period of each 1.778ms symbol is modulated by 32 pulses of a 36kHz carrier
//					The 36KHz carrier signal has a duty factor of 0.25.
//					(i.e. high for 6.944us and then low for 20.833us; a repetition period of 27.777us)

// Data Output		The command and system address are placed into the registers "command" and "system".
//					The toggle bit is placed into bit 6 of "command" register
//					(this is used to differentiate between a new key press of the same key,
//					and a duplicate transmission of the key value (because it auto-repeats))
//					If an error is detected in the signal decoding 0xFF is placed into "system" and "command"
//
//					This code only detects the lower 64 control codes (of 128 possible codes)
//					Therefore, some buttons on the remote handset will not be detected

// RC5 code numerical values (decimal)
//					RC5Code_0 = 0
//					RC5Code_1 = 1
//					RC5Code_2 = 2
//					RC5Code_3 = 3
//					RC5Code_4 = 4
//					RC5Code_5 = 5
//					RC5Code_6 = 6
//					RC5Code_7 = 7
//					RC5Code_8 = 8
//					RC5Code_9 = 9
//					NTL control - UP arrow (Page up (^)) = 10
//					NTL control - Down arrow (Page down) = 11
//					NTL control - Power On/Off = 12
//					NTL control - Settings = 14
//					NTL control - Help = 15
//					RC5Code_VOL_UP = 16
//					RC5Code_VOL_DOWN = 17
//					RC5Code_HASH = 28
//					NTL control - favourites = 29
//					RC5Code_CHAN_UP = 32
//					RC5Code_CHAN_DOWN = 33
//					NTL control - Back = 40
//******************************************************************
#define MAXIMUM_CommandCodeValue 0x3F // 6-bit code

#define INPUT_PIN_MASK 0b00000100	// RC5 signal on bit 2 of PORT D
#define INT2_Handler_TRIGGERED 1
#define INT2_Handler_IDLE 0

#include <avr/io.h>
#include <avr/interrupt.h>

unsigned char INT2_Handler_State;
unsigned char FineIntervalCount;
unsigned char CoarseTimer;
unsigned char FineTimer;
unsigned char BitCount;
unsigned char ref1, ref2;

void InitialiseGeneral(void);
void Initialise_TIMER2_CTC_64uS();
void Initialise_INT2(void);
void ENABLE_INT2(void);

unsigned char RC5_receive(void);
void Delay_1500_uS(void);
void Delay_16_uS(void);

unsigned char RC5_Command_code;
unsigned char RC5_System_code;

int main(void)
{
	RC5_Command_code = 0;
	INT2_Handler_State = INT2_Handler_IDLE;
	Initialise_TIMER2_CTC_64uS();
	Initialise_INT2();
	InitialiseGeneral();
	ENABLE_INT2();

	while(1)
	{
		if(INT2_Handler_TRIGGERED == INT2_Handler_State)
		{
			RC5_receive();
			if(0xFF != RC5_Command_code)
			{	// If a valid command code has been decoded, Display the command code

				// Uncomment the following line to remove the toggle bit (bit 6)
				//RC5_Command_code  &= 0b00111111;
				PORTB = ~RC5_Command_code;

				PORTC = RC5_System_code;
			}
			INT2_Handler_State = INT2_Handler_IDLE;
			ENABLE_INT2();
		}
	}
}

void InitialiseGeneral()
{
	DDRB = 0xFF;			// Configure PortB direction for Output
	PORTB = 0xFF;			// Set all LEDs initially off (inverted on the board, so '1' = off)
	DDRC = 0xFF;			// Configure PortC direction for Output
	PORTC = 0x00;			// Set all LEDs initially off

	DDRD = 0x00;			// Port D Data Direction Register (Port D set for input)
							// IR Receiver is connected to bit 2 of PORTD

	sei();	// Enable interrupts at global level, set Global Interrupt Enable (I) bit
}

void Initialise_INT2()
{
	// EICRA – External Interrupt Control Register A
	// Bits 7:0 – ISC31, ISC30 – ISC00, ISC00: External Interrupt 3 - 0 Sense Control Bits
	// 10  = The falling edge of INTn generates asynchronously an interrupt request
	EICRA = 0b00100000;		// Interrupt Sense (INT2) falling-edge triggered

	// EICRB – External Interrupt Control Register B
	// Bits 7:0 – ISC71, ISC70 - ISC41, ISC40: External Interrupt 7 - 4 Sense Control Bits
	// 10  = The falling edge of INTn generates asynchronously an interrupt request
	EICRB = 0b00000000;

	// EIMSK – External Interrupt Mask Register
	// Bits 7:0 – INT7:0: External Interrupt Request 7 - 0 Enable
	EIMSK = 0b00000000;		// Initially disabled, set bit 2 to Enable H/W Int 2

	// EIFR – External Interrupt Flag Register
	// Bits 7:0 – INTF7:0: External Interrupt Flags 7 - 0
	EIFR = 0b11111111;		// Clear all HW interrupt flags (in case a spurious interrupt has occurred during chip startup)
}

void ENABLE_INT2()
{
	EIMSK = 0b00000100;		// Enable H/W Int 2
}

ISR(INT2_vect)		// Interrupt Handler for INT2
{
	// Disable needs to be done quickly, so done 'in-line' and not as function call
	EIMSK = 0b00000000;		// Disable INT2 whilst receiving the RC5 code (same I/O pin is used)
	INT2_Handler_State = INT2_Handler_TRIGGERED;	// Set flag, keep Interrupt handler as short as possible
}

void Initialise_TIMER2_CTC_64uS()
{
	// TCCR2A –Timer/Counter Control Register A
	// Bits 7:6 – COM2A1:0: Compare Match Output A Mode
	// Bits 5:4 – COM2B1:0: Compare Match Output B Mode
	// Bits 3, 2 – Res: Reserved Bits
	// Bits 1:0 – WGM21:0: Waveform Generation Mode   (010 = CTC)
	TCCR2A = 0b00000010;	// CTC

	// TCCR2B – Timer/Counter Control Register B
	// Bit 7 – FOC2A: Force Output Compare A
	// Bit 6 – FOC2B: Force Output Compare B
	// Bits 5:4 – Res: Reserved Bits
	// Bit 3 – WGM22: Waveform Generation Mode
	// Bit 2:0 – CS22:0: Clock Select (001 = prescaler 1, 100 = prescaler 64)
	TCCR2B = 0b00000100;	// CTC, prescaler 64

	//Timer/Counter count/value register TCNT2
	TCNT2 = 0x00;

	//Set Output Compare Register (OCR2)
	OCR2A = 0b00000000;		// 0 Due to the way the timer is reset in CTC mode
							// Timed interval is 64uS with 1MHz clock and clkIO/64 prescaling)
							// (The reset operation is detected on the following timeout)

	// Asynchronous Status Register (ASSR)
	// This is read-only, except for bits 6 and 5
	// Bit 6 – EXCLK: Enable External Clock Input
	// Bit 5 – AS2: Asynchronous Timer/Counter2
	ASSR = 0b00000000;

	// TIMSK2 – Timer/Counter2 Interrupt Mask Register
	// Bit 2 – OCIE2B: Timer/Counter2 Output Compare Match B Interrupt Enable
	// Bit 1 – OCIE2A: Timer/Counter2 Output Compare Match A Interrupt Enable
	// Bit 0 – TOIE2: Timer/Counter2 Overflow Interrupt Enable
	TIMSK2 = 0b00000010;	// Enable OCIE2A interrupt
}

ISR(TIMER2_COMPA_vect)		// Compare A Interrupt Handler for Timer 2
{
	FineTimer++;		//Increment the fine-grained time interval count every 64uS
	FineIntervalCount++;
	if(0 == FineIntervalCount)
	{	// Here if rolled over, so may need to check if 0 UC
		CoarseTimer++;	//Increment coarse counter, every 256th interrupt (every 16,384uS (16mS))
	}
}

unsigned char RC5_receive(void) // RC5 code is 14 bits long, only return the 6-bit command code
{	// Read 14 bits {2 start bits (0b11), 1 control bit (alternates), 5 system bits, 6 command bits }
detect:
	FineIntervalCount = 0;
	CoarseTimer = 0;

detect1:
	FineTimer = 0;

detect2:
	if(CoarseTimer >= 8)
	{
		goto fault;
	}

dl1:
	if(FineTimer >= 55)
	{
		goto start1;
	}
	if(PIND & INPUT_PIN_MASK)
	{	// Bit is a '1'
		goto detect2;
	}
	else
	{
		goto detect1;
	}

start1:
	if(CoarseTimer >= 8)
	{
		goto fault;
	}
	if(PIND & INPUT_PIN_MASK)
	{	// Bit is a '1'
		goto start1;
	}
	FineTimer = 0;

start2:
	if(FineTimer >= 17)
	{
		goto fault;
	}
	if(PIND & INPUT_PIN_MASK)
	{	// Bit is a '1'
		ref1 = (FineTimer * 3) / 2; // FineTimer is 1/2 a bit-time, ref1 = 3/4 bit-time
		ref2 = (FineTimer * 5) / 2; // ref2 = 5/4 bit-time
		FineTimer = 0;
	}
	else
	{
		goto start2;
	}

start3:
	if(FineTimer >= ref1)
	{
		goto fault;
	}
	if(PIND & INPUT_PIN_MASK)
	{	// Bit is a '1'
		goto start3;
	}
	else
	{
		FineTimer = 0;
		BitCount = 12;	// Count bits, expecting 12
		RC5_Command_code = 0;
		RC5_System_code = 0;
	}

sample:
	if(FineTimer <= ref1)
	{
		goto sample;
	}
	if(PIND & INPUT_PIN_MASK)
	{	// Bit is a '1'
		goto bit_is_a_1;
	}

bit_is_a_0:	// A data bit '0' has been detected
	if(RC5_Command_code & 0b10000000)
	{
		RC5_System_code = (RC5_System_code*2) +1;
	}
	else
	{
		RC5_System_code *=2;
	}
	RC5_Command_code *=2;

bit_is_a_0a:	// Synchronize timing
	if(FineTimer >= ref2)
	{
		goto fault;
	}
	if(PIND & INPUT_PIN_MASK)
	{	// Bit is a '1'
		FineTimer = 0;
		goto nextbit;
	}
	else
	{
		goto bit_is_a_0a;
	}

bit_is_a_1: // A data bit '1' has been detected
	if(RC5_Command_code & 0b10000000)
	{
		RC5_System_code = (RC5_System_code*2) +1;
	}
	else
	{
		RC5_System_code *=2;
	}
	RC5_Command_code = (RC5_Command_code*2) +1;

bit_is_a_1a:	// Synchronize timing
	if(FineTimer >= ref2)
	{
		goto fault;
	}
	if(PIND & INPUT_PIN_MASK)
	{	// Bit is a '1'
		goto bit_is_a_1a;
	}
	else
	{
		FineTimer = 0;
	}

nextbit:
	BitCount--;
	if(BitCount > 0)
	{
		goto sample;
	}

	if(RC5_Command_code & 0b10000000)
	{
		RC5_System_code = (RC5_System_code*2) +1;
	}
	else
	{
		RC5_System_code *=2;
	}
	if(RC5_Command_code & 0b01000000)
	{
		RC5_System_code = (RC5_System_code*2) +1;
	}
	else
	{
		RC5_System_code *=2;
	}
	if(RC5_System_code & 0b00100000)
	{
		RC5_Command_code |= 0b01000000;
	}

	RC5_Command_code &= 0b01111111;	// Toggle, + 6 'command' bits
	RC5_System_code &= 0b00011111;	// 5 'address' bits
	goto detect_end;

fault:	// Set both "command" and "system" to 0xFF to indicate failure
	RC5_Command_code = 0xFF;
	RC5_System_code = 0xFF;

detect_end:
	return RC5_Command_code;
}