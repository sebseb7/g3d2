#  Project Name
PROJECT=test
#  List of the objects files to be compiled/assembled
OBJECTS=core/startup_LPC17xx.o core/core_cm3.o core/system_LPC17xx.o main.o
LSCRIPT=core/ldscript_rom_gnu.ld

OPTIMIZATION = 0
DEBUG = -g
#LISTING = -ahls

#  Compiler Options
GCFLAGS = -Wall -fno-common -mcpu=cortex-m3 -mthumb -O$(OPTIMIZATION) $(DEBUG)
GCFLAGS += -D__RAM_MODE__=0
GCFLAGS += -Wa,-adhlns=$@.lst
#GCFLAGS += -Wcast-align -Wcast-qual -Wimplicit -Wpointer-arith -Wswitch
#GCFLAGS += -Wredundant-decls -Wreturn-type -Wshadow -Wunused
LDFLAGS = -mcpu=cortex-m3 -mthumb -O$(OPTIMIZATION) -nostartfiles  -T$(LSCRIPT)
#LDFLAGS = -mcpu=cortex-m3 -mthumb -O$(OPTIMIZATION) -nostartfiles -Wl,-Map=bin/$(PROJECT).map -T$(LSCRIPT)
ASFLAGS = $(LISTING) -mcpu=cortex-m3 --defsym RAM_MODE=0

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
	$(REMOVE) *.o.lst core/*.o.lst

#########################################################################
#  Default rules to compile .c and .cpp file to .o
#  and assemble .s files to .o

.c.o : Makefile
	$(GCC) $(GCFLAGS) -o $@ -c $<


.S.o : Makefile
	$(AS) $(ASFLAGS) -o $(PROJECT)_crt.o $< > $(PROJECT)_crt.lst

#########################################################################


flash: all
	lpc21isp -verify $(PROJECT).hex /dev/ttyUSB0 19200 14746

