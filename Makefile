DEVICE  = atmega2560
F_CPU   = 16000000

CC = avr-gcc
CFLAGS = -std=c99 -Wall -Os -mmcu=$(DEVICE) -DF_CPU=$(F_CPU)ULL -I.

OBJ = main.o serial.o

main.hex: main.elf
	rm -f main.hex
	avr-objcopy -R .eeprom -O ihex main.elf main.hex
	avr-size main.elf

main.elf: $(OBJ)
	$(CC) $(CFLAGS) -o main.elf -Wl,-Map,main.map,--gc-sections $(OBJ)
#	$(CC) $(CFLAGS) -o main.elf -Wl,-Map,main.map,-u,vfprintf -lprintf_min $(OBJ)

%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	rm -f $(OBJ) main.elf main.hex main.bin

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
