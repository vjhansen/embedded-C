# ls -a /dev/tty*

# PORT 				= /dev/cu.usbmodemBUR1846711382
PORT 					= /dev/tty.usbmodem14201
FILENAME 		= adc_main
DEVICE 			= atmega2560
#DEVICE 			= atmega328p
#PROGRAMMER 	= arduino
PROGRAMMER 		= wiring
BAUD 				= 115200
COMPILE 		= avr-gcc -Wall -pedantic -mmcu=$(DEVICE) -F_CPU=1000000UL -g -Os


default: compile upload clean

compile:
	$(COMPILE) -c $(FILENAME).c -o $(FILENAME).o
	$(COMPILE) -o $(FILENAME).elf $(FILENAME).o
	avr-objcopy -j .text -j .data -O ihex $(FILENAME).elf $(FILENAME).hex
	avr-size --format=avr --mcu=$(DEVICE) $(FILENAME).elf

upload:
	avrdude -v -F -V -D -p $(DEVICE) -c $(PROGRAMMER) -P $(PORT) -b $(BAUD) -U flash:w:$(FILENAME).hex:i

clean:
	rm $(FILENAME).o
	rm $(FILENAME).elf
