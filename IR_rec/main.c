
/*  - - - - - - - - - - - - - - - - -
    -  main.c
    -  Author: Victor J. Hansen
    -  Update: 01.12.2020
    
    *  Contains functions for receiving and sending NEC IR Protocol in "raw" and standard format with 16 bit Address  8bit Data
    
    https://dronebotworkshop.com/using-ir-remote-controls-with-arduino/
    https://github.com/electrobs/AVR-LIBRARY-IR_REMOTE_RECV
    The IR-remote uses the NEC protocol. 
    https://techdocs.altium.com/display/FPGA/NEC+Infrared+Transmission+Protocol
*/

/*
   - - - - - - - - - - - - - - - - -
   IR rx <-> Arduino Mega (Connection)
   - - - - - - - - - - - - - - - - -
   VCC     <-> 5V
   GND     <-> GND
   OUT     <-> PL0 = ICP4 = 49 on board
*/


// AVR libraries
#include <avr/io.h>
#include <avr/interrupt.h>

// C libs
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define F_CPU 16000000UL  // 16 MHz

#include <util/delay.h>


#define CR  0x0D
#define LF  0x0A // Line feed

enum FSM { WAIT, CHECK, DONE }; 

void USART0_SETUP_9600_BAUD();
void USART0_TX_SingleByte(unsigned char cByte);
void USART0_TX_String(char* sData);
void InitialiseGeneral();
void timer1();
void init_timer4();

// vars for ultrasonic sensor
volatile unsigned int elapsedTime, transmitTime;
static unsigned char dataCnt = 0;
volatile unsigned char timer3_cnt, elapseCnt = 0;
volatile float spaceTime, startTime, endTime;
volatile unsigned char rising, falling = 0;

unsigned char state = WAIT;

static char textToWrite[32]  = {'\0'};
static char receivedData[10] = {'\0'};

// - - - - - - - - - - - - - - - - -
int main()
{
    InitialiseGeneral();
    timer1();
    init_timer4();
    USART0_SETUP_9600_BAUD();
    
    while(1)
    {
        /* if (strlen(receivedData > 6)) { */
        /*     USART0_TX_String(receivedData); */
        /*     USART0_TX_String("\n"); */
        /* } */
        
        switch (state)
        {
// - - - - - - - - - - - - - - - - -
        case WAIT:
            for (int i = 0; i < 10; ++i) { // clear output
                receivedData[i] = '\0';
            }
            if (falling == 1) {
                TCNT1H = 0x00;  // Timer/Counter count/value registers (16 bit)
                TCNT1L = 0x00;
                _delay_ms(31.5);
                state=CHECK;
            }
            break;
// - - - - - - - - - - - - - - - - - 
        case CHECK:
            if (falling == 1)
            {
                /* Logical '0' – a 562.5µs pulse burst followed by a 562.5µs space, 
                   with a total transmit time of 1.125ms */

                /* Logical '1' – a 562.5µs pulse burst followed by a 1.6875ms space,
                   with a total transmit time of 2.25ms */

                // Logic 0
                if (spaceTime <= 564 && spaceTime > 562)
                {
                    receivedData[dataCnt] = '0';
                    dataCnt++;
                }
                // Logic 1
                else if (spaceTime > 1686 && spaceTime <= 1688) // 3*563
                {
                    receivedData[dataCnt] = '1';
                    dataCnt++;
                }
            }
            else if (dataCnt > 7)
            {
                dataCnt = 0;
                state = DONE;
            }
            break;
// - - - - - - - - - - - - - - - - - 
        case DONE:
            if (elapseCnt == 13)
            {
                // USART0_TX_String("complete \n\r");
                state=WAIT;
            }
            break;
        default:
            break;
        }
    }
}

// - - - - - - - - - - - - - - - - - 
void InitialiseGeneral()
{   
    asm ("sei"); // Enable interrupts
}

// timer for total elapsed time
// Timer1 (16 bits)
void timer1()
{
    TCCR1A = 0x00;
    TCCR1B = (1<<WGM12 | 1<<CS11);  // CTC waveform mode, prescaler = 8
    
    // 4.5 msec -> ((4.5 msec * 16 MHz ) / 8 prescaler) = 0x2328
    
    OCR1AH = 0x23; // Output Compare Registers (16 bit)
    OCR1AL = 0x28;

    TCNT1H = 0x00;  // Timer/Counter count/value registers (16 bit)
    TCNT1L = 0x00;
    TIMSK1 = (1<<OCIE1A);  // generate an interrupt when the timer reaches the set value in the OCR1A register
}


ISR (TIMER1_COMPA_vect)
{
    // 40.5 ms - 9 = 31.5 ms
    // 67.5 ms - 9 = 58.5 ms
    elapseCnt++;
    if (elapseCnt > 14)
    {
        elapseCnt = 0;
    }
}

// get rising edge of signal
void init_timer4()
{
    TCCR4A = 0x00;
    
    // Input capture on falling edge / prescaler 64
    TCCR4B = (1<<WGM42 | 1<<CS41 | 1<<CS40);

    // (16 MHz / 64) / 1,000,000 counts/uSec = 1/4 counts/us
    // 4 us per count 
    
    TIMSK4 = (1<<ICIE4); // Input Capture Interrupt Enable

    // set top value for counter
    // 50 ms -> ((0.05 s * 16 MHz ) / 64 prescaler) = 0x30d4
    OCR4AH = 0x30;
    OCR4AL = 0xd4;
}

// get space time, period from falling edge until rising edge

/* 
   When a change of the logic level (an event) occurs on the Input Capture Pin (ICPn), 
   and this change confirms to the setting of the edge detector, a capture will be triggered. 
   When a capture is triggered, the 16-bit value of the TCNTn is written to the Input Capture Register (ICRn). 
*/

/* 
   Reading the 16-bit value in the Input Capture Register (ICRn) is done by first reading the low byte (ICRnL) and
   then the high byte (ICRnH). When the low byte is read the high byte is copied into the high byte Temporary 
   Register (TEMP). When the CPU reads the ICRnH I/O location it will access the TEMP Register. 
*/

/* 
   The ICRn Register can only be written when using a Waveform Generation mode that utilizes the ICRn Register for 
   defining the counter’s TOP value. In these cases the Waveform Generation mode (WGMn3:0) bits must be set before 
   the TOP value can be written to the ICRn Register. When writing the ICRn Register the high byte must be written to 
   the ICRnH I/O location before the low byte is written to ICRnL. 
 */


ISR (TIMER4_CAPT_vect)
{
    if (TCCR4B & (1<<ICES4)) // rising edge
    {
        TCCR4B &= ~(1<<ICES4);
        // Next time detect falling edge (ICESn = 0)
        rising = 1;
        falling = 0;
        endTime = ICR4;
        spaceTime = endTime-startTime;
        //   sprintf(textToWrite, "%f" , spaceTime);
    }
    else  // Falling edge
    {
        TCCR4B |= (1<<ICES4); // Next time detect rising edge (ICESn = 1)
        startTime = ICR4; // Save current count
        rising = 0;
        falling = 1;
    }
}


// screen /dev/cu.usbserial 9600
// press Ctrl+a, type :quit and press Enter. 

void USART0_SETUP_9600_BAUD()
{
    // USART Control and Status Register A
    UCSR0A = (1<<U2X0); // Double the USART Transmission Speed
    // Writing this bit to one will reduce the divisor of the baud rate divider from 16 to 8 effectively doubling the transfer
    //rate for asynchronous communication.

    // USART Control and Status Register B
    /* RX Complete Interrupt Enable, RX Enable, TX Enable, 8-bit data */
    UCSR0B = (1<<RXCIE0 | 1<<RXEN0 | 1<<TXEN0);
    
    // USART Control and Status Register C
    /* Asynchronous, No Parity, 1 stop, 8-bit data, Falling XCK edge */
    UCSR0C = (1<<UCSZ01 | 1<<UCSZ00 | 1<<UCPOL0);
  
    // UBRR0 - USART Baud Rate Register (16-bit register, comprising UBRR0H and UBRR0L)
    UBRR0H = 0; // 9600 baud, UBRR = 12, and  U2X must be set to '1' in UCSRA
    UBRR0L = 207;
}

void USART0_TX_SingleByte(unsigned char cByte)
{
    while( !(UCSR0A & (1 << UDRE0)) ); // Wait for Tx Buffer to become empty (check UDRE flag)
    UDR0 = cByte;   // Writing to the UDR transmit buffer causes the byte to be transmitted
}

void USART0_TX_String(char* sData)
{
    int iCount;
    int iStrlen = strlen(sData);
    if(0 != iStrlen)
    {
        for(iCount = 0; iCount < iStrlen; iCount++)
        {
            USART0_TX_SingleByte(sData[iCount]);
        }
        USART0_TX_SingleByte(CR);
        USART0_TX_SingleByte(LF);
    }
}


/* ISR(USART0_RX_vect) // USART Receive-Complete Interrupt Handler */
/* { */
/*     char cData = UDR0; */
/*     if (cData == 'd') */
/*     { */
/*         /\* for (int i = 0; i < 8; i++) *\/ */
/*         /\* { *\/ */
/*         /\*     USART0_TX_SingleByte(receivedData[i]); *\/ */
/*         /\* } *\/ */
       

/*     } */
/* } */

/* void decodeIR(char *IRdata) */
/* { */
/*     switch */
// if IRdata = 
// -> 8 on remote

/* } */
