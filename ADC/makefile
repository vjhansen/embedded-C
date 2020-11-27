# ls -a /dev/tty*

# PORT 				= /dev/cu.usbmodemBUR1846711382
PORT 					= /dev/ttyACM0
FILENAME 		= adc_main
DEVICE 			= atmega2560
#DEVICE 			= atmega328p
#PROGRAMMER 	= arduino
PROGRAMMER 		= wiring
BAUD 				= 115200
COMPILE 		= avr-gcc -mmcu=$(DEVICE)

# E: 0xFD, H: 0x50, L: 0x62
default: compile upload clean

compile:
	$(COMPILE) -c $(FILENAME).c -o $(FILENAME).o
	$(COMPILE) -o $(FILENAME).elf $(FILENAME).o
	avr-objcopy -j .text -j .data -O ihex $(FILENAME).elf $(FILENAME).hex
	avr-size --format=avr --mcu=$(DEVICE) $(FILENAME).elf

upload:
	avrdude  -p $(DEVICE) -c $(PROGRAMMER) -P $(PORT) -u  -U flash:w:$(FILENAME).hex:i -v -D



clean:
	rm $(FILENAME).o
	rm $(FILENAME).elf
