
/*  - - - - - - - - - - - - - - - - -
    -  main.c
    -  Author: Victor J. Hansen
    -  Update: 08.11.2020
*/


/* 
   - - - - - - - - - - - - - - - - -
   LCD functionality is based on:
   LCD_1602A_FixedMessage_PortsAG_2560_C (Richard Anthony, 16th October 2016)
   - - - - - - - - - - - - - - - - -
   
   - - - - - - - - - - - - - - - - -
   RGB-LED <-> Arduino Mega (Connection)
   - - - - - - - - - - - - - - - - -
   VCC     <-> 5V
   GND     <-> GND
   R       <-> PK0
   G       <-> PK1
   B       <-> PK2
   - - - - - - - - - - - - - - - - -
   Buzzer  <->  PK4
*/

/*
  - - - - - - - - - - - - - - - - -
  Ultrasonic sensor HC-SR04
  Datasheet: 
  https://cdn.sparkfun.com/datasheets/Sensors/Proximity/HCSR04.pdf
  Range: 2 cm - 400 cm
  Accuracy: ~3 mm
  Ultrasonic sensor HC-SR04 <-> Arduino Mega (Connection)
  - - - - - - - - - - - - - - - - -
  VCC    <-> 5V
  GND    <-> GND
  ECHO   <-> PL0 (ICP4)
  TRIG   <-> PL1
  - - - - - - - - - - - - - - - - -
*/


// AVR libraries
#include <avr/io.h>
#include <avr/interrupt.h>

// C libs
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// 1 MHz clk
#define F_CPU 1000000UL

#include <util/delay.h>

// header files
#include "LCD_Lib_2560.h"
#include "keypad.h"
#include "usart_2560.h"

#define TopRow       0
#define BottomRow    1
#define R_LED        PK0
#define G_LED        PK1
#define B_LED        PK2
#define BUZZER       PK4
#define TRIGpin      PINL1

// - Function declarations
void InitialiseGeneral();
void init_timer1();
void init_timer3();
void init_timer4();
void init_timer5();
void pressing_keypad(unsigned char KeyValue);

/*
  A volatile modifier is used when we want to prevent 
  a variable from being allocated to an AVR register. 
  This warns the compiler that the var may be subject to external changes. 
  We declare data types as volatile if they are global or gets interrupted by a timer.
*/

// vars for ultrasonic sensor
volatile int16_t counts, dist, rising, falling, us_per_count;
volatile unsigned char set_intruder_flag = 0;

// vars for USART
volatile unsigned char textToWrite[16];
volatile unsigned char hyperText[16];

// vars for keypad and passcode
volatile unsigned char passcode[8] = {1,2,3,4};
volatile unsigned char KeyPresses = 0;
volatile unsigned char first_digit, second_digit, third_digit, fourth_digit;
volatile unsigned char set_disarm_flag, set_passcode_flag = 0;
volatile unsigned char new_passcode_flag = 0;

// used for testing
volatile unsigned char ElapsedSeconds_Count = 0;

// - - - - - - - - - - - - - - - - -
int main()
{
    unsigned char KeyValue;
    InitialiseGeneral();
    LCD_Home();
    init_timer1();
    init_timer3();
    init_timer4();
    init_timer5();
    USART0_SETUP_9600_BAUD();

    while(1)
    {
        KeyValue = ScanKeypad();
        // user is setting a new passcode
        if (new_passcode_flag == 1 && set_passcode_flag == 1)
        {
            for (int i=0; i<4; i++)
            {
                USART0_TX_SingleByte(passcode[i]+48);   // print current passcode
                _delay_ms(250);
            }
            USART0_TX_String("\n");
        }
        // pressing keypad
        if (NoKey != KeyValue)
        {
            pressing_keypad(KeyValue);
        }
        // correct passcode
        else if (first_digit == passcode[0]
                 && second_digit == passcode[1]
                 && third_digit  == passcode[2]
                 && fourth_digit == passcode[3]
                 && KeyPresses != 0)
        {
            TCNT5H = 0x00;  // start 1 min counter
            TCNT5L = 0x00;
            // testing
            // TCNT1H = 0x00; 
            // TCNT1L = 0x00;
            set_disarm_flag = 1; // disarm system
            set_passcode_flag = 0;
        }
        // stay disarmed for 1 minute
        while (set_disarm_flag == 1)
        {
            PORTK = (1<<G_LED); // green LED on
            LCD_Display_ON_OFF(true, false, false);
            LCD_Clear();
            LCD_WriteString("DISARMED");
            _delay_ms(1000);
            KeyPresses = 0;
            set_passcode_flag = 0;
        }
        // while system is armed: detect movement in the range [5 cm, 35 cm]
        if (dist > 4 && dist < 36 && set_disarm_flag == 0 && set_intruder_flag == 0)
        {
            TCNT3H = 0x00;
            TCNT3L = 0x00;
            set_intruder_flag = 1;
        }
        // activate alarm
        if (set_intruder_flag == 1 && set_passcode_flag == 0)
        {
            PORTK = (1<<R_LED | 1<<BUZZER); // red LED, buzzer active
            LCD_Display_ON_OFF(true, false, false);
            LCD_Clear();
            LCD_SetCursorPosition(0, TopRow);
            LCD_WriteString("OBJECT");
            LCD_SetCursorPosition(0, BottomRow);
            LCD_WriteString("DETECTED");
            _delay_ms(750);
        }
        // standby
        if (KeyPresses == 0)
        {
            first_digit = second_digit = third_digit = fourth_digit = 0;
            PORTK = 0x00; // no alarm
            _delay_ms(700);
            LCD_Display_ON_OFF(false, false, false);
        }
    }
}

void InitialiseGeneral()
{   
    /*  RGB-LED + buzzer  */
    DDRK |=  (1<<BUZZER | 1<<B_LED | 1<<G_LED | 1<<R_LED); // Configure PortK direction for Output
    PORTK &= ~(1<<BUZZER | 1<<B_LED | 1<<G_LED | 1<<R_LED); // Set all LEDs + buzzer initially off

    /*  SONAR */
    DDRL |= (1<<TRIGpin); // set TRIGPIN as output
    PORTL &= ~(1<<TRIGpin); // set low

    /*  KeyPad  */
    DDRC = 0xF0; // Row pins output / Column pins input
    PORTC = 0x0F; // Set pull-ups on column pins (so they read '1' when no key is pressed)

    /*  LCD */
    LCD_Initilise(true, false);
    LCD_ShiftDisplay(false, true);
    LCD_Display_ON_OFF(true, false, false);
    // Display a fixed message on the top and bottom row of the LCD
    LCD_Clear();
    LCD_Home();
    LCD_SetCursorPosition(0, TopRow);
    LCD_WriteString("CWK-ESP5200");
    LCD_SetCursorPosition(0, BottomRow);
    LCD_WriteString("Victor Hansen");
    _delay_ms(5000);
    
    asm ("sei"); // Enable interrupts
}

// used for testing
// generate an interrupt after a 1-Second interval
void init_timer1()
{
    TCCR1A = 0x00;  // Normal port operation (OC1A, OC1B, OC1C), Clear Timer on 'Compare Match' (CTC) waveform mode)
    TCCR1B = (1<<WGM12 | 1<<CS12 | 1<<CS10);  // CTC waveform mode, initially stopped (no clock), prescaler = 1024
    
    // 1 MHz clock w/ 1024 prescaler
    // For 1 sec we need to count to ( (1 sec * 1000000 MHz) / 1024) = 0x03D0
    OCR1AH = 0x03; // Output Compare Registers (16 bit)
    OCR1AL = 0xD0;

    TCNT1H = 0x00;  // Timer/Counter count/value registers (16 bit)
    TCNT1L = 0x00;
    TIMSK1 = (1<<OCIE1A);  // generate an interrupt when the timer reaches the set value in the OCR1A register
}

// used for testing (1 s counter)
ISR(TIMER1_COMPA_vect) // TIMER1_CompareA_Handler (Interrupt Handler for Timer 1)
{
    ElapsedSeconds_Count++;
    if (set_disarm_flag == 0) {ElapsedSeconds_Count =0;}
    sprintf(hyperText, "%d", ElapsedSeconds_Count);
//    USART0_TX_String("\r\n");
//    USART0_TX_String(hyperText);
}

/*  Ultrasonic Sensor  */
void init_timer4()
{
    TCCR4A = (1<<WGM41); // Clear OCnA/OCnB/OCnC on compare match (set output to low level)
    TIMSK4 = (1<<OCIE4A | 1<<ICIE4);
    
    // Input Capture Noise Canceler / Input capture on rising edge / prescaler 8
    TCCR4B = (1<<ICNC4 | 1<<ICES4 | 1<<CS41);
    
    /*  ICESn selects which edge on the Input Capture pin (ICPn) that is used to trigger a capture event.
        ICESn bit = 0 --> a falling edge is used as trigger.
        ICESn bit = 1 --> a rising edge will trigger the capture.
        When a capture is triggered, the counter value is copied into the Input Capture Register (ICRn).
    */
    
    /*
      The datasheet for HCSR04 suggest to use over 60 ms measurement cycle.
      70ms measurement cycle: 1MHz/8 = 125k counts/sec -> 12,5k counts/100ms 
      -> 12500/100*70 = 8750 counts per 70ms
    */
    
    OCR4AH = 0x22;
    OCR4AL = 0x2e;
    us_per_count = 8; /* (1MHz / 8) = 125,000 counts per second 
                         -> 125,000/1,000,000 counts per usec = 1/8 counts per us
                         -> 8 us per count */ 
}

ISR (TIMER4_CAPT_vect)
{
    /*  Rising edge (signal on the ICP pin goes from 0 -> 1): 
        switch ICP to falling edge detection and then store the 'start-time'.
       
        Falling edge (1 -> 0): switch ICP to rising edge detection and get the 'end-time' to 
        calculate the distance in cm.
    */
    if (TCCR4B & (1<<ICES4)) // Rising edge
    {
        TCCR4B &= ~(1<<ICES4); // Next time detect falling edge (ICESn = 0)
        rising = ICR4; // Save current count (start-time)
    }
    else  // Falling edge
    {
        TCCR4B |= (1<<ICES4); // Next time detect rising edge (ICESn = 1)
        falling = ICR4; // Save current count (end-time)
        counts = falling-rising;
        dist = (us_per_count*counts)/(59);  // (usec/(2*29.4 usec/cm)) to get distance in cm
        // distance-string, used for diagnostics
        sprintf(textToWrite, "%d", dist);
    }
}

/*
  We need to supply a short 10 uS pulse to the trigger input (TRIGpin) to start the ranging.
  Then the module will send out an 8 cycle burst of ultrasound at 40 kHz and raise its echo. 
  - From data-sheet (Ultrasonic Ranging Module HC-SR04)
*/
ISR (TIMER4_COMPA_vect)
{
    PORTL |= (1<<TRIGpin);
    _delay_us(10);
    PORTL &= ~(1<<TRIGpin);
}

void init_timer3() // Configure to generate an interrupt after a 10 and 30 seconds
{
    TCCR3A = 0x00;  // Normal port operation (OC1A, OC1B), Clear Timer on 'Compare Match' (CTC) waveform mode)
    TCCR3B = (1<<WGM32 | 1<<CS32 | 1<<CS30);  // CTC waveform mode, use prescaler 1024

    // 20 sec -> ((20 sec * 1 MHz ) / 1024 prescaler) = 0x4c4b
    OCR3AH = 0x4c;
    OCR3AL = 0x4b;

    // 30 sec -> ((30 sec * 1 MHz ) / 1024 prescaler) = 0x7271
    OCR3BH = 0x72;
    OCR3BL = 0x71;

    TCNT3H = 0x00;  // Timer/Counter count/value registers (16 bit) TCNT3H and TCNT3L
    TCNT3L = 0x00;
    TIMSK3 = (1<<OCIE3B | 1<<OCIE3A);  /* Use 'Output Compare (A, B) Match' Interrupt, i.e. generate an interrupt
                                          when the timer reaches the set value (in the OCR3(A, B) registers) */
}

// Interrupt Handlers for Timer 3
ISR(TIMER3_COMPA_vect) { set_intruder_flag = 0; }
ISR(TIMER3_COMPB_vect) { set_passcode_flag = 0; }

void init_timer5()     // Configure to generate an interrupt after a 60 Second interval
{
    TCCR5A = 0x00;  // Normal port operation (OC1A, OC1B, OC1C), Clear Timer on 'Compare Match' (CTC) waveform mode)
    TCCR5B =  (1<<WGM52 | 1<<CS52 | 1<<CS50); // CTC waveform mode, prescaler=1024

    // ((60 sec * 1000000 MHz ) / 1024 prescaler) = 0xe4e2
    OCR5AH = 0xe4;
    OCR5AL = 0xe2;

    // Timer/Counter count/value registers
    TCNT5H = 0x00;
    TCNT5L = 0x00;
    TIMSK5 = (1<<OCIE5A); // 'Output Compare A Match' Interrupt
}

ISR(TIMER5_COMPA_vect) { set_disarm_flag = 0; }

ISR(USART0_RX_vect) // USART Receive-Complete Interrupt Handler
{
    char cData = UDR0;
    // set new passcode
    if (cData == 'p')
    {
        USART0_TX_String("\nSet new passcode:\r\n");
        new_passcode_flag = 1;
        TCNT3H = 0x00;  // Timer/Counter count/value registers
        TCNT3L = 0x00;
        set_passcode_flag = 1;
    }
    // done setting passcode
    else if (cData == 'q')
    {
        USART0_TX_String("\nPasscode set\r\n");
        new_passcode_flag = 0;
        set_passcode_flag = 0;
        _delay_ms(500);
    }
    // print distance (diagnostic) 
    else if (cData == 'd')
    {
        new_passcode_flag = 0;
        USART0_TX_String("\nDistance in cm: \r\n");
        USART0_TX_String(textToWrite);
        _delay_ms(500);
    }
}

// function for handling the values entered on the keypad
void pressing_keypad(unsigned char KeyValue)
{
    _delay_ms(100);
    KeyPresses++;
    _delay_ms(100); // debounce delay

    if (KeyValue == 16) // enter passcode
    {
        TCNT3H = 0x00; // reset timer3 counters
        TCNT3L = 0x00;
        set_passcode_flag = 1; // allow the user to enter a passcode
        set_intruder_flag = 0; // intruder flag
        KeyPresses = 0;
        new_passcode_flag = 0;
        LCD_Display_ON_OFF(true, false, false);
        LCD_Clear();
        LCD_SetCursorPosition(0,TopRow);
        LCD_WriteString("Enter passcode: ");
        _delay_ms(1000);
    }
    else if (KeyValue == 13) // soft-reset in case the user does not want to enter a passcode
    {
        PORTK = 0x00;
        LCD_Clear();
        set_passcode_flag = 0; // user does not enter a passcode
        KeyValue = NoKey;
    }
    else if (new_passcode_flag == 1 && KeyPresses < 5) // setting new passcode via USART and keypad
    {
        _delay_ms(100);
        PORTK = (1<<B_LED); // blue LED
        passcode[KeyPresses-1] = KeyValue;
        _delay_ms(100);
    }
    else if (set_passcode_flag == 1) // the user has 30 seconds to enter a 4-digit passcode
    {
        PORTK = (1<<B_LED);
        // get first digit of code
        if (KeyPresses == 1) 
        {
            first_digit = KeyValue;
            LCD_Display_ON_OFF(true, false, false);
            LCD_Clear();
            LCD_SetCursorPosition(1,TopRow);
            LCD_WriteString("*"); // indicate that user has entered a single digit
            _delay_ms(750);
        }
        else if (KeyPresses == 2)  // 2 of 4 digits of passcode entered
        {
            second_digit = KeyValue;
            LCD_Clear();
            LCD_SetCursorPosition(1,TopRow);
            LCD_WriteString("**");
            _delay_ms(750);
        }
        else if (KeyPresses == 3) // 3 of 4 digits of passcode entered
        {
            third_digit = KeyValue;
            LCD_Clear();
            LCD_SetCursorPosition(1,TopRow);
            LCD_WriteString("***");
            _delay_ms(750);
        }
        else if (KeyPresses == 4)  // 4 of 4 digits of passcode entered
        {
            fourth_digit = KeyValue;
            LCD_Clear();
            LCD_SetCursorPosition(1,TopRow);
            LCD_WriteString("****");
            _delay_ms(750);
        }
        else if (KeyPresses > 4) // only allow 4 digit passcodes
            KeyPresses = 0;
    }
}
