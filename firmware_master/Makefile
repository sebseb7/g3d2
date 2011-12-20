PROJECT=g3d2_master


ifeq ($(OSTYPE),)
OSTYPE      = $(shell uname)
endif
ifneq ($(findstring Darwin,$(OSTYPE)),)
USB_DEVICE = $(shell ls /dev/cu.usbserial*)
else
USB_DEVICE = /dev/ttyUSB0
endif

LSCRIPT=core/ldscript_rom_gnu.ld

OPTIMIZATION = 2
DEBUG = -g

#########################################################################

SRC=$(wildcard core/*.c *.c drivers/*.c libs/*.c) pixelfonts/5x3/font.c
ASRC=$(wildcard core/*.s)
OBJECTS= $(SRC:.c=.o) $(ASRC:.s=.o)
HEADERS=$(wildcard core/*.h drivers/*.h libs/*.h *.h) pixelfonts/5x3/font.h

#  Compiler Options
GCFLAGS = -std=gnu99 -Wall -fno-common -mcpu=cortex-m3 -mthumb -O$(OPTIMIZATION) $(DEBUG) 
# -ffunction-sections -fdata-sections -fmessage-length=0   -fno-builtin
GCFLAGS += -D__RAM_MODE__=0 
LDFLAGS = -mcpu=cortex-m3 -mthumb -O$(OPTIMIZATION) -nostartfiles  -T$(LSCRIPT) 
ASFLAGS = -mcpu=cortex-m3 --defsym RAM_MODE=0

#  Compiler/Assembler Paths
GCC = arm-none-eabi-gcc
AS = arm-none-eabi-as
OBJCOPY = arm-none-eabi-objcopy
REMOVE = rm -f
SIZE = arm-none-eabi-size

#########################################################################

all: $(PROJECT).hex Makefile stats

$(PROJECT).hex: $(PROJECT).elf Makefile
	$(OBJCOPY) -R .stack -O ihex $(PROJECT).elf $(PROJECT).hex

$(PROJECT).elf: $(OBJECTS) Makefile
	$(GCC) $(LDFLAGS) $(OBJECTS) -o $(PROJECT).elf

stats: $(PROJECT).elf Makefile
	$(SIZE) $(PROJECT).elf

clean:
	$(REMOVE) $(OBJECTS)
	$(REMOVE) $(PROJECT).hex
	$(REMOVE) $(PROJECT).elf

#########################################################################

%.o: %.c Makefile $(HEADERS)
	$(GCC) $(GCFLAGS) -o $@ -c $<

%.o: %.s Makefile 
	$(AS) $(ASFLAGS) -o $@  $< 

#########################################################################

flash: all
	lpc21isp -verify $(PROJECT).hex  $(USB_DEVICE) 19200 14746

