#  Project Name
PROJECT=g3d2_master

#  List of the objects files to be compiled/assembled
SRC=$(wildcard core/*.c *.c)
ASRC=$(wildcard core/*.s)
OBJECTS= $(SRC:.c=.o) $(ASRC:.s=.o)

LSCRIPT=core/ldscript_rom_gnu.ld



OPTIMIZATION = 1
DEBUG = -g

USB_DEVICE = /dev/ttyUSB0

#  Compiler Options
GCFLAGS = -Wall -fno-common -mcpu=cortex-m3 -mthumb -O$(OPTIMIZATION) $(DEBUG)
GCFLAGS += -D__RAM_MODE__=0
#GCFLAGS += -Wa,-adhlns=$@.lst
#GCFLAGS += -Wcast-align -Wcast-qual -Wimplicit -Wpointer-arith -Wswitch
#GCFLAGS += -Wredundant-decls -Wreturn-type -Wshadow -Wunused
LDFLAGS = -mcpu=cortex-m3 -mthumb -O$(OPTIMIZATION) -nostartfiles  -T$(LSCRIPT)
ASFLAGS = -mcpu=cortex-m3 --defsym RAM_MODE=0

#  Compiler/Assembler/Linker Paths
PATH:=/usr/local/Sourcery_G++_Lite/bin:${PATH}
export PATH
GCC = arm-none-eabi-gcc
AS = arm-none-eabi-as
LD = arm-none-eabi-ld
OBJCOPY = arm-none-eabi-objcopy
REMOVE = rm -f
SIZE = arm-none-eabi-size

#########################################################################

all:: $(PROJECT).hex Makefile stats


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
#	$(REMOVE) *.o.lst core/*.o.lst

#########################################################################
#  Default rules to compile .c and .cpp file to .o
#  and assemble .s files to .o

%.o: %.c Makefile
	$(GCC) $(GCFLAGS) -o $@ -c $<

%.o: %.s Makefile
	$(AS) $(ASFLAGS) -o $@  $< 

#########################################################################

flash: all
	lpc21isp -verify $(PROJECT).hex  $(USB_DEVICE) 19200 14746

