/*
 * Cycle_SampleProgramForDebugPractice.c
 *
 * Created: 20/01/2012 21:37:52
 * Author: James Hawthorne
 */ 

#include <avr/io.h>

unsigned char ledValue = 1;

int main(void)
{
	DDRB = 0xFF;
	PORTB = 0xFF;
	
    while(1)
    {
        PORTB = ~ledValue;
		ledValue *= 2;
		
		if (ledValue == 0)
			ledValue = 1;
		Delay();
    }
}

void Delay()
{
	unsigned char DelayCount1, DelayCount2, DelayCount3;	// Declare local variables
	
	for(DelayCount1 = 0X04; DelayCount1 > 0; DelayCount1--)
	{
		for(DelayCount2 = 0XFF; DelayCount2 > 0; DelayCount2--)
		{
			for(DelayCount3 = 0XFF; DelayCount3 > 0; DelayCount3--);
		}
	}
}