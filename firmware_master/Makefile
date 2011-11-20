PROJECT=g3d2_master

USB_DEVICE = /dev/ttyUSB0

LSCRIPT=core/ldscript_rom_gnu.ld

OPTIMIZATION = 1
DEBUG = -g

#########################################################################

SRC=$(wildcard core/*.c *.c)
ASRC=$(wildcard core/*.s)
OBJECTS= $(SRC:.c=.o) $(ASRC:.s=.o)

#  Compiler Options
GCFLAGS = -Wall -fno-common -mcpu=cortex-m3 -mthumb -O$(OPTIMIZATION) $(DEBUG)
GCFLAGS += -D__RAM_MODE__=0
LDFLAGS = -mcpu=cortex-m3 -mthumb -O$(OPTIMIZATION) -nostartfiles  -T$(LSCRIPT)
ASFLAGS = -mcpu=cortex-m3 --defsym RAM_MODE=0

#  Compiler/Assembler Paths
PATH:=/usr/local/Sourcery_G++_Lite/bin:${PATH}
export PATH
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

%.o: %.c Makefile
	$(GCC) $(GCFLAGS) -o $@ -c $<

%.o: %.s Makefile
	$(AS) $(ASFLAGS) -o $@  $< 

#########################################################################

flash: all
	lpc21isp -verify $(PROJECT).hex  $(USB_DEVICE) 19200 14746

