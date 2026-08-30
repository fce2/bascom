MCU  = atmega328p
PORT = COM6
ifeq ($(wildcard ../bin/win/abc.exe),)
BC   = ../bin/win/abcd.exe
else
BC   = ../bin/win/abc.exe
endif
AC   = ../../../../../../tools/avr-gcc/avr-gcc-15.2.0-x86-windows/bin

.PHONY: all clean

all: breath.elf #morse.elf

%.elf: %.bas
	$(BC) $*.bas --avr=$(MCU)
	$(AC)/avr-gcc.exe -I ../.. -mmcu=$(MCU) -Os -ffunction-sections -fdata-sections -DB_TIER=2 -DF_CPU=16000000UL -DBAUD=115200 -Wl,--gc-sections -Wl,-Map,$*.map $*.c -lm -o $*.elf
	$(AC)/avr-objcopy.exe -O ihex $*.elf $*.hex
	$(AC)/avr-objcopy.exe -O binary $*.elf $*.bin
	$(AC)/avr-objdump.exe -d $*.elf > $*.asm
	$(AC)/avrdude.exe -C $(AC)/avrdude.conf -c arduino -p $(MCU) -P $(PORT) -b 115200 -U flash:w:$*.hex

clean:
	rm -f *.asm *.bin *.c *.elf *.hex *.map
