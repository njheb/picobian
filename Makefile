# microbian/Makefile
# Copyright (c) 2020 J. M. Spivey

include config.mk

EXAMPLES = ex-level.bin ex-valentine.bin ex-echo.bin ex-remote.bin \
	ex-timeout.bin

all: microbian.a startup.o

examples: $(EXAMPLES)

ex-level.elf: accel.o

CPU = -mcpu=$(CHIP) -mthumb
CFLAGS = -O -g -Wall -ffreestanding -I $(BOARD)
CC = arm-none-eabi-gcc
AS = arm-none-eabi-as
AR = arm-none-eabi-ar

vpath %.c $(BOARD)

#DRIVERS = timer.o serial.o i2c.o radio.o display.o adc.o
DRIVERS =

MICROBIAN = microbian.o $(MPX).o $(DRIVERS) lib.o

microbian.a: $(MICROBIAN)
	$(AR) cr $@ $^

ex-%.bin: $(BIN_PREPEND) ex-unpadded-%.bin
	cat $^ >$@

ex-%.uf2: ex-%.bin uf2
	./uf2 $< $@

# Note that this rule intentionally uses system cc.
uf2: uf2.c
	cc $< -o $@

ex-unpadded-%.bin: ex-%.elf
	arm-none-eabi-objcopy -O binary $< $@

%.elf: %.o startup.o microbian.a
	$(CC) $(CPU) $(CFLAGS) -T $(LSCRIPT) -nostdlib $^ -lc -lgcc \
		-o $@ -Wl,-Map,$*.map

%.o: %.c
	$(CC) $(CPU) $(CFLAGS) -c $< -o $@ 

%.o: %.s
	$(AS) $(CPU) $< -o $@

%.h: %.f
	./hwdesc $< >$@

clean: force
	rm -f microbian.a *.o *.elf *.bin *.map $(BOARD)/*.o $(BOARD)/*.bin uf2

force:

###

$(MICROBIAN) startup.o: microbian.h lib.h $(BOARD)/hardware.h
