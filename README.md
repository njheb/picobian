# microbian
The micro:bian embedded OS for BBC micro:bit and other
ARM-based microcontroller boards.

This repository exists primarily to store the full version of
micro:bian, including timeouts and ports to other boards: so that the
version distributed with my book, _Bare Metal micro:bit_, can be
simplified as much as possible.

To build, link make a symbolic link named `config.mk` to either `config.v1`
or `config.v2`, then use `make` or `make examples`.

## Pi Pico

To build for the Raspberry Pi Pico, link `config.mk` to `config.pico`. Then, use
`make` to generate a `uf2` file for the appropriate example using, for instance,
`make ex-echo.uf2`. This will compile `uf2.c` using the native `cc` and use it
to generate a UF2 file which can be directly flashed to the chip by the standard
process of mounting the Pico as a filesystem and copying the image to it.
`uf2.c` is written in standard C and should function on any little-endian host.

There is also a dependency on Python 3 in this process: a Python script is used
to pad and checksum the stage2 bootloader which is put into the image.

## Picobian 

Progress - wip check in, lots still to do

Examples:

`ex-timeout.c` added BUTTON_A and BUTTON_B, note there are a block of debug pins following,
shows how to configure internal pull-up and configure input to use with momentary switch to ground.

`ex-adc.c` uses adc driver to report values from a potentiometer.

`ex_i2c_blocking.c` just enough code for an i2c0 oled display, not using a proper driver, using pins out of the way of those debug pins (should get around to documenting mlugg's debug pins better than this hint).



Focus on Raspi pico but other RP2040 targets like qtpy2040 and xiao2040
should be ok too so change name back when ready to push to parent.

Have added systick supported timer driver.

Have added adc polling driver - need to looking into FCS FIFO irq

Have rudiments of polling i2c0 driver, supports ssd1306 128x32 i2c display by default
with addition of a single `#define __DISP_64__` should work with 128x64 i2c display, x64 untested.
 
Future direction:
Looks like linux drivers for the i2c fabric exist in separate
master and slave versions. So they should provide a guide to getting
a proper interrupt driven model going for i2c0 and i2c1

search internet for:

`.../linux/drivers/i2c/busses/i2c/i2c-designware-master.c`

`.../linux/drivers/i2c/busses/i2c/i2c-designware-slave.c`



Fixup adc driver to use interrupts to.

After that spi driver is required.

May also investigate Serial CDC using tinyusb however the mutex and semaphore 
primatives will need to be added, have not investigated adding to RP2040.


