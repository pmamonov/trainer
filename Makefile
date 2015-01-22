DEVICE  = atmega32u4
F_CPU   = 16000000

LUFA_PATH = .
LUFA_OPTS  = -D USB_DEVICE_ONLY
LUFA_OPTS += -D FIXED_CONTROL_ENDPOINT_SIZE=8
LUFA_OPTS += -D FIXED_NUM_CONFIGURATIONS=1
LUFA_OPTS += -D USE_FLASH_DESCRIPTORS
LUFA_OPTS += -D USE_STATIC_OPTIONS="(USB_DEVICE_OPT_FULLSPEED | USB_OPT_REG_ENABLED | USB_OPT_AUTO_PLL)"
LUFA_OPTS += -DINTERRUPT_CONTROL_ENDPOINT
include $(LUFA_PATH)/LUFA/makefile
LUFA_SRC = $(LUFA_SRC_USB) $(LUFA_SRC_USBCLASS)

CC = avr-gcc
CFLAGS =  -std=c99 -Wall -Os -mmcu=$(DEVICE) -DF_CPU=$(F_CPU)UL -I. \
-DF_USB=$(F_CPU)UL -I$(LUFA_PATH) $(LUFA_OPTS)

OBJ=main.o\
 $(LUFA_SRC:%.c=./%.o) Descriptors.o

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
	avrdude -c avr109 -P /dev/ttyACM0 -p m32u4 -b 57600 -U flash:w:main.hex:i
#	$(batchisp) -hardware usb -device $(DEVICE) -operation erase f
#	$(batchisp) -hardware usb -device $(DEVICE) -operation loadbuffer `pwd`/main.hex program
#	$(batchisp) -hardware usb -device $(DEVICE) -operation start reset 0
#	sudo avrdude -P usb -c usbasp -p m32 -U flash:w:main.bin:r

#fuse:
#	sudo avrdude -P usb -c usbasp -p m32 -U lfuse:w:lfuse.bin:r -U hfuse:w:hfuse.bin:r

