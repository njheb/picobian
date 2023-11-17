#!/usr/bin/env python3

import sys
import binascii
import struct

"""
This script is responsible for taking the generated stage2 bootloader blob,
padding it to the required size, and appending the checksum. The RP2040
stage1 bootloader requires the loaded blob to be exactly 256 bytes in size
with a correct checksum.

Usage: pad_stage2 <input filename> <output filename>
"""

PADDED_LENGTH = 256

def bitrev(x, width):
    return int("{:0{w}b}".format(x, w=width)[::-1], 2) # TODO this is dumb

if len(sys.argv) != 3:
    print(f"Usage: {sys.argv[0]} <input file> <output file>")
    sys.exit(1)

in_file = sys.argv[1]
out_file = sys.argv[2]

with open(in_file, 'rb') as f:
    data_in = f.read()

# Save 4 bytes for checksum
data_padded = data_in + bytes(PADDED_LENGTH - len(data_in) - 4)

# This logic was taken from the Raspberry Pi Pico SDK (licensed as BSD-3-Clause).
# The checksum is weirdly manipulated to ultimately save bytes in the checking
# logic in the stage1 bootloader.
raw_checksum = binascii.crc32(bytes(bitrev(b, 8) for b in data_padded), 0)
checksum = bitrev((raw_checksum ^ 0xffffffff) & 0xffffffff, 32)

data_out = data_padded + struct.pack("<L", checksum)

with open(out_file, 'wb') as f:
    f.write(data_out)
