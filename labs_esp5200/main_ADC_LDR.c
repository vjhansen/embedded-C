//*****************************************
// Project   ADC Light Dependant Resistor (LDR) Demonstration (ADC)
// Program   ADC_LightDependentResistor_2560_LabSkeleton.c
// Author    Richard Anthony - 20th September 2019



// Function      Reads ADC channel 0 (Port F bit 0)
//          Displays binary digital value on LEDs
//          Also sends data via USART0 for display on Embedded Systems Workbench (ESWB)
//          Data transmitted at 10Hz - uses Timer1 to generate interrupts at 10Hz 

//          *** Important note regarding connecting to the Embedded Systems Workbench:
//            If you encounter problems getting the Embedded Systems Workbench to auto-detect the Atmel,
//            you can just 'open' the appropriate port inside the 'Serial Port configuration' dialog box
//            (by selecting it, usually it is the last one on the 'Serial Ports list') and then close the
//            'Serial Port configuration' dialog box and open the graph display dialog window.
  
//          ADC operates in 'free run mode'
//          (Auto re-starts conversion at end of previous conversion)
//          Triggers interrupt when conversion is complete

// I/O hardware    Uses LDR and 10K Ohm resistor bridge circuit connected to Port F bit 0
//          Circuit is:  GND (0V) ---10K Ohm resistor--- ADC bit 0 ---LDR--- 5V)

// I/O Ports
//          USART0 (PortE) is used to connect to PC (application is Embedded Systems Workbench)
//          Port F bit 0 is used as an analogue input for the Analogue-to-Digital converter
//          Port K is set for output (LEDs)

/*
Part B: Determine what light-value reading corresponds to the level at which you would need 
  to turn the lights on in a room. This value will be used as a threshold to control a simulated lamp.

From ESWB:
DARK (covering LDR with hand) ~ 115
NORMAL = 207-208

*/

#include <avr/io.h>
#include <avr/interrupt.h>
#include "USART_ESWB_AtmelSide_2560_LIBRARY.h"

#define dark_threshold 115

void InitialiseGeneral();
void InitialiseTimer1_10Hz();
void InitialiseTimer3();
void Initialise_ADC();
void Start_ADC_Conversion(void);

volatile unsigned char one_min_flag;

int main(void)
{
  InitialiseGeneral();
  InitialiseTimer1_10Hz();
  InitialiseTimer3();
  Initialise_ADC();
  
  // Configure interface to ESWB
  USART0_SETUP_9600_BAUD_ASSUME_1MHz_CLOCK();  // In USART_ESWB_AtmelSide_2560_LIBRARY Library
  Reset_Connection_State();          // In USART_ESWB_AtmelSide_2560_LIBRARY Library
  g_uOperationMode = MODE_3_SEND_DATA;
 
  g_uData = 128;                // Initialise to midrange (will be overwritten by ADC sample values)
  // change this to a value > than the dark_threshold if the on-board led lights up without you covering the LDR
  
  Start_ADC_Conversion();    // Start ADC conversion (only need to do this the first time
                // because 'free running' mode is used)
                
  while(1) {
    if (g_uData <= dark_threshold)
    {
      TCNT3H = 0x00;  // Timer/Counter count/value registers (16 bit) TCNT3H and TCNT3L
      TCNT3L = 0x00;
      one_min_flag = 1;
    }        
    else 
      PORTB &= ~(1 << PINB7); // turn off on-board LED
    
    while (one_min_flag == 1) {
      PORTB |= (1 << PINB7); // turn on on-board LED
    }
  }
  
}

void InitialiseGeneral()
{  
  DDRB |= (1 << PINB7);
  DDRK  = 0xFF;  // Configure PortK direction for Output to LEDs
  PORTK = 0x00;  // Set all LEDs initially off  
  sei();      // Enable interrupts at global level set Global Interrupt Enable (I) bit
}

void InitialiseTimer1_10Hz()  // Configure to generate an interrupt at 20Hz (i.e. 50ms interval)
{
  TCCR1A = 0x00;      // Normal port operation (OC1A, OC1B, OC1C), Clear Timer on 'Compare Match' (CTC) waveform mode)
  TCCR1B = 0b00001010;  // CTC waveform mode, prescaler 8
  TCCR1C = 0x00;
  
  // For 1 MHz clock (with 8 prescaler) to achieve a 100ms second interval:
  // Need to count 100,000 clock cycles (100,000 / 8 = 12,500)
  OCR1A = 12500; // Count to 12500

  TCNT1H = 0x00;  // Timer/Counter count/value registers (16 bit) TCNT1H and TCNT1L
  TCNT1L = 0x00;
  TIMSK1 = 0b00000010;  // bit 1 OCIE1A: Use 'Output Compare A Match' Interrupt, i.e. generate an interrupt
  // when the timer reaches the set value (in the OCR1A registers)
}

ISR(TIMER1_COMPA_vect) // TIMER0_Overflow_Handler (Interrupt Handler for Timer 0)
{
  // Send data value (the output of the A-to-D converter) to the ESWB
  // The data value in g_uData is transmitted every time the interrupt occurs  
  USART0_TX_SingleByte(g_uData);
}


void InitialiseTimer3()     // Configure to generate an interrupt after a 60 Second interval
{
  TCCR3A = 0x00;  // Normal port operation (OC1A, OC1B, OC1C), Clear Timer on 'Compare Match' (CTC) waveform mode)
  TCCR3B = 0b00001101;    // CTC waveform mode, use prescaler 1024
  TCCR3C = 0x00;
  
  // For 1 MHz clock (with 1024 prescaler) to achieve a 60 second interval:
  // Need to count 60*1,000,000 clock cycles (but already divided by 1024)
  // So actually need to count to (60*1000000 / 1024) = 58594 decimal, = E4E2 Hex
  //OCR3AH = 0xE4; // Output Compare Registers (16 bit) OCR1BH and OCR1BL
  //OCR3AL = 0xE2;
  
  // 5 sec = 0x1313  --- used for debugging
  OCR3AH = 0x13;
  OCR3AL = 0x13;

  TCNT3H = 0x00;  // Timer/Counter count/value registers (16 bit) TCNT3H and TCNT3L
  TCNT3L = 0x00;
  TIMSK3 = 0b00000010;    // bit 1 OCIE3A     Use 'Output Compare A Match' Interrupt, i.e. generate an interrupt
  // when the timer reaches the set value (in the OCR3A registers)
}

ISR(TIMER3_COMPA_vect) // TIMER3_Overflow_Handler (Interrupt Handler for Timer 3)
{
  one_min_flag = 0;
}

void Initialise_ADC()
{
  // Set ADC multiplexer selection register (ADMUX)
  // Bit 7:6 - REFS1:0: Reference Selection Bits
  //    01 = AVCC with external capacitor at AREF pin
  
  // Bit 5 - ADLAR: ADC Left Adjust Result
  //  0 = ADCH (high) contains bit 1 = output bit 9, bit 0 = output bit 8
  //    ADCL (low) contains output bits 7 through 0
  //  1 = ADCH (high) contains bits 9 through 2
  //      ADCL (low) contains bit 7 = output bit 1, bit 6 = output bit 0
  
  // Bits 4:0 - MUX4:0: Analog Channel and Gain Selection Bits
  //  00000 = ADC0 (ADC channel 0, single-ended input)
  //  (When bits 4 and 3 are both 0, bits 2-0 indicate which ADC channel is used; single-emnded input)
  //  00010 = ADC2 (ADC channel 2, single-ended input)
  ADMUX = 0b01100000;    // Use AVCC as voltage ref, left adjust (need 8 MSBs) Convert channel 0 (LDR connected to bit 0)

  //Enable and set up ADC via ADC Control and Status Register(ADCSR)
  //(note, header file discrepancy: ADCSR register is named ADCSRA in the header file)
  //  Bit 7 - ADEN: ADC Enable = 1
  //  Bit 6 - ADSC: ADC Start Conversion = 0
  //  Bit 5 - ADATE: ADC Auto Trigger Enable = 1
  //  Bit 4 - ADIF: ADC Interrupt Flag = 0
  //  Bit 3 - ADIE: ADC Interrupt Enable = 1
  //  Bits 2:0 - ADPS2:0: ADC Prescaler Select Bits = 110 (division factor 64)
  ADCSRA = 0b10101110; // ADC enabled, Auto trigger, Interrupt enabled, Prescaler = 64

  // ADCSRB - ADC Control and Status Register B
  // Bit 2:0 - ADTS2:0: ADC Auto Trigger Source = 000 (free running mode)
  // ADCSRB = 0b11110000;    // clear bits 3,2,1,0 (Free running mode)
  ADCSRB &= ~((1 << ADTS2) | (1 << ADTS1) | (1 << ADTS0));

  // DIDR0 - Digital Input Disable Register 0
  //  Bit 7:0 - ADC7D:ADC0D: ADC7:0 Digital Input Disable
  // DIDR0 = 0b00000001;  // Disable digital input on bit 0
  DIDR0 |= (1 << ADC0D);

  // DIDR2 - Digital Input Disable Register 2
  //  Bit 7:0 - ADC15D:ADC8D: ADC15:8 Digital Input Disable
  DIDR2 = 0xFF;  // Disable digital input on all bits
}

void Start_ADC_Conversion(void)
{  // Start the ADC Conversion (start first sample, runs in 'free run' mode after)
  //  bit 6 ADSC (ADC Start Conversion) = 1 (START)
  //  Read ADSCSR and OR with this value to set the flag without changing others
  unsigned char New_ADCSRA;
  New_ADCSRA = ADCSRA | 0b01000000;
  ADCSRA = New_ADCSRA;
}

ISR(ADC_vect)  // ADC Interrupt Handler
{
  // After the conversion is complete, the conversion result can be found in the ADC Result Registers (ADCL, ADCH).
  g_uData = ADCH;   // Copy the output data from the A-to-D converter to a global variable
  PORTK = g_uData;  // Display the output data from the A-to-D converter onto the LEDs
}
