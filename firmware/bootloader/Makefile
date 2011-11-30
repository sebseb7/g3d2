PROJECT=g3d2_bl

MCU = atmega88pa
MCU_AVRDUDE = atmega88pa


BOOTSIZE=1024

MT_BOOTLOADER_ADDRESS = 0x1800
FUSE_SETTINGS = -U lfuse:w:0xD6:m -U hfuse:w:0xDC:m -U efuse:w:0xF8:m -U lock:w:0xEF:m


ifeq ($(OSTYPE),)
OSTYPE      = $(shell uname)
endif
ifneq ($(findstring Darwin,$(OSTYPE)),)
USB_DEVICE = /dev/cu.SLAB_USBtoUART
else
USB_DEVICE = /dev/ttyUSB0
endif


#########################################################################

SRC=$(wildcard *.c)
OBJECTS=$(SRC:.c=.o) 
DFILES=$(SRC:.c=.d) 
HEADERS=$(wildcard *.h)



#  Compiler Options
GCFLAGS = -mmcu=$(MCU) -I. -gstabs -DBOOTSIZE=$(BOOTSIZE)  -Os -funsigned-char -funsigned-bitfields -fpack-struct -fshort-enums -Wall -Wstrict-prototypes  -std=gnu99 -MD -MP
#LDFLAGS =  -Wl,-Map=pwbl.map,--cref    -lm -Wl,--section-start=.text=0x1800
LDFLAGS = -mmcu=$(MCU)   -Wl,--section-start=.text=$(MT_BOOTLOADER_ADDRESS)

#  Compiler Paths
GCC = avr-gcc
REMOVE = rm -f
SIZE = avr-size
OBJCOPY = avr-objcopy

#########################################################################

all: $(PROJECT).hex Makefile stats

$(PROJECT).hex: $(PROJECT).elf Makefile
	$(OBJCOPY) -R .eeprom -O ihex $(PROJECT).elf $(PROJECT).hex 

$(PROJECT).elf: $(OBJECTS) Makefile
	$(GCC) $(LDFLAGS) $(OBJECTS) -o $(PROJECT).elf

stats: $(PROJECT).elf Makefile
	$(SIZE) $(PROJECT).elf

clean:
	$(REMOVE) $(OBJECTS)
	$(REMOVE) $(PROJECT).hex
	$(REMOVE) $(DFILES)
	$(REMOVE) $(PROJECT).elf

#########################################################################

%.o: %.c Makefile $(HEADERS)
	$(GCC) $(GCFLAGS) -o $@ -c $<

#########################################################################

flash: all
	avrdude -p $(MCU_AVRDUDE) -P $(USB_DEVICE) -c stk500v2    -U flash:w:$(PROJECT).hex

fuse:
	avrdude -p $(MCU_AVRDUDE) -P $(USB_DEVICE) -c stk500v2    $(FUSE_SETTINGS)
