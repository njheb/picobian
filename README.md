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
