DEVICE  = atmega2560
F_CPU   = 16000000

CC = avr-gcc
CFLAGS = -std=c99 -Wall -Os -mmcu=$(DEVICE) -DF_CPU=$(F_CPU)ULL -I.

OBJ = main.o bt.o serial.o

main.hex: main.elf
	rm -f main.hex
	avr-objcopy -R .eeprom -O ihex main.elf main.hex
	avr-size main.elf

main.elf: $(OBJ)
	$(CC) $(CFLAGS) -o main.elf -Wl,-Map,main.map,--gc-sections $(OBJ)
#	$(CC) $(CFLAGS) -o main.elf -Wl,-Map,main.map,-u,vfprintf -lprintf_min $(OBJ)

%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

generator.elf: generator.c
	$(CC) -Os -mmcu=atmega32 -DF_CPU=12000000ull -o $@ $<

generator.hex: generator.elf
	rm -f generator.hex
	avr-objcopy -R .eeprom -O ihex generator.elf generator.hex
	avr-size generator.elf

clean:
	rm -f $(OBJ) main.elf main.hex main.bin generator.elf generator.hex

load:
	avrdude -c usbasp -p m2560 -U flash:w:main.hex:i
#	$(batchisp) -hardware usb -device $(DEVICE) -operation erase f
#	$(batchisp) -hardware usb -device $(DEVICE) -operation loadbuffer `pwd`/main.hex program
#	$(batchisp) -hardware usb -device $(DEVICE) -operation start reset 0
#	sudo avrdude -P usb -c usbasp -p m32 -U flash:w:main.bin:r

#fuse:
#	sudo avrdude -P usb -c usbasp -p m32 -U lfuse:w:lfuse.bin:r -U hfuse:w:hfuse.bin:r

disablebootloader:
	avrdude -c usbasp -p m2560 -U hfuse:w:0xd9:m

loadgen: generator.hex
	avrdude -F -c usbasp -p m32 -U flash:w:generator.hex:i

