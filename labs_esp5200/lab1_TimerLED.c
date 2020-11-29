//*******************************************************************************
// Project Timer Demo1 Single-Timer Single-LED-Flash (Embedded C)
// Target ATMEL ATmega2560 micro-controller on Arduino 2560 Mega board
// Program TimerDemo1_SingleTimerSingleLEDFlash_2560_C.c

 // Function         Demonstrates how to use the programmable timers
 // Flashes a single LED at 1Hz (1 flash per second)
 // Also shows the use of an Interrupt Handler
 // The red on-board LED on Port B bit 7 is used to show the output of the program


// PART A


#include <avr/io.h>
#include <avr/interrupt.h>

// Declare functions (these will be in a separate header file in larger programs)
void InitialiseGeneral();
void InitialiseTimer1();

// Declare global variables
volatile unsigned char uLED_value; // An 'unsigned char' is an 8-bit numeric value, i.e. single byte

int main(void) {
  InitialiseGeneral();
  uLED_value = 0x00; // Initialise the variable
  InitialiseTimer1();

  while(1) {}
}

void InitialiseGeneral() {
  DDRB = 0xFF; // Configure PortB direction for Output
  PORTB = 0x00; // Set all bits to 0 (ensures the LED is initially off
}

void InitialiseTimer1() { // Configure to generate an interrupt after a 1-Second interval
  TCCR1A = 0x00; /* Normal port operation (OC1A, OC1B, OC1C), Clear Timer on 'Compare
  Match' (CTC) waveform mode) */

  TCCR1B = 0b00001101; // CTC waveform mode, use prescaler 1024
  TCCR1C = 0x00;

  // For 1 MHz clock (with 1024 prescaler) to achieve a 1 second interval:
  // Need to count 1 million clock cycles (but already divided by 1024)
  // So actually need to count to (1000000 / 1024 =) 976 decimal, = 3D0 Hex
  OCR1AH = 0x03; // Output Compare Registers (16 bit) OCR1BH and OCR1BL
  OCR1AL = 0xD0;

  TCNT1H = 0x00; // Timer/Counter count/value registers (16 bit) TCNT1H and TCNT1L
  TCNT1L = 0x00;

  TIMSK1 = 0b00000010; /* bit 1 OCIE1A Use 'Output Compare A Match' Interrupt,
  i.e. generate an interrupt when the timer reaches the set value
  (in the OCR1A register) */

  sei(); // Enable interrupts at global level, set Global Interrupt Enable (I) bit
}


ISR(TIMER1_COMPA_vect) { // TIMER1_CompareA_Handler (Interrupt Handler for Timer 1)
      // Flip the value of the least significant bit of the 8-bit variable
      if(0 == uLED_value) {
        uLED_value = 1;
        PORTB = 0x80; // Turn on LED (bit 7)
      }
      else {
        uLED_value = 0;
        PORTB = 0x00; // Turn off LED (bit 7)
      }
}


