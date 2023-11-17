// Second-stage bootloader.
//
// Rather than using flash XIP, the bootloader simply copies all code from flash
// into RAM, before loading the vector table offset into VTOR, loading the stack
// pointer, and jumping to the entry point.
//
// This binary blob will be padded to 256 bytes (including a trailing 4-byte
// checksum). Thus, the code cannot exceed 252 bytes.

// We do not include SRAM5 here, since that is where this bootloader
// is loaded. See corresponding logic in main linker script.
.equ SRAM_BASE,  0x20000000
.equ SRAM_END,   0x20041000
.equ FLASH_BASE, 0x10000000

.equ XIP_SSI_BASE, 0x18000000
// SPI_FRF = standard 1-bit frame format
// DFS_32 = 32 clocks per frame
// TMOD = EEPROM read mode
.equ XIP_CTRLR0_VAL, ((0 << 21) | (31 << 16) | (3 << 8))
// XIP_CMD = CMD_READ
// INST_L = 8-bit instruction
// ADDR_L = 24-bit addresses
// TRANS_TYPE = command and address in standard format
.equ XIP_SPI_CTRLR0_VAL, ((3 << 24) | (2 << 8) | (6 << 2) | (0 << 0))
.equ SCB_VTOR_ADDR, 0xe000ed08

.section .text

// The actual bootloader entrypoint.
_boot_entry:
	bl _enable_xip
	bl _do_memcpy
	bl _disable_xip
	// The binary is now loaded at SRAM_BASE.
	// Load the vector table offset into VTOR, set up the stack pointer, and jump
	// to the entry point.
	ldr r0, =SRAM_BASE // vector table is at the start of the image
	ldr r1, =SCB_VTOR_ADDR
	str r0, [r1]
	ldmia r0, {r0, r1}
	msr msp, r0
	bx r1

_test_led:
	ldr r0, =0x4000f000
	mov r1, #32
	str r1, [r0]
rst:
	ldr r0, =0x4000c008
	ldr r1, [r0]
	cmp r1, #0
	beq rst
	ldr r0, =0x400140cc   // IO_BANK0_GPIO25_CTRL
	mov r1, #5
	str r1, [r0]
	mov r1, #1
	lsl r1, r1, #25
	ldr r0, =0xd0000024   // SIO_GPIO_OE_SET
	str r1, [r0]
	ldr r0, =0xd0000014   // SIO_GPIO_OUT_SET
	str r1, [r0]
loop:
	b loop

// Enable XIP to the point where we can read (but not necessarily execute!) from
// flash memory at FLASH_BASE.
_enable_xip:
	push {lr}

	ldr r0, =XIP_SSI_BASE

	mov r1, #0
	str r1, [r0, #0x08] // SSIENR = 0 (disable SSI)

	mov r1, #4 // clock divisor
	str r1, [r0, #0x14] // BAUDR.SCKDV = 4

	ldr r1, =XIP_CTRLR0_VAL
	str r1, [r0, #0x00] // CTRLR0

	mov r1, #0
	str r1, [r0, #0x04] // CTRLR1.NDF = 0

	ldr r1, =XIP_SPI_CTRLR0_VAL
	// This offset is too big to encode as an actual offset!
	ldr r2, =(XIP_SSI_BASE + 0xf4) // SPI_CTRLR0
	str r1, [r2]

	mov r1, #1
	str r1, [r0, #0x08] // SSIENR = 1 (enable SSI)

	pop {pc}

// Disable XIP again when we're done with it.
_disable_xip:
	push {lr}
	ldr r0, =XIP_SSI_BASE
	mov r1, #0
	str r1, [r0, #0x08] // SSIENR
	pop {pc}

// Fill RAM from SRAM_BASE through SRAM_END with data from flash memory,
// starting from FLASH_BASE.
_do_memcpy:
	push {lr}

	// Conveniently, the bootrom contains efficient implementations of some SDK
	// functions. We're going to be copying a large amount of data, so an
	// efficient memcpy routine is helpful here!

	// Get a pointer to the function used to lookup the routine in the table
	mov r2, #0x18
	ldrh r2, [r2]
	// Get a pointer to the table itself
	mov r0, #0x14
	ldrh r0, [r0]
	// This code (ASCII 'C','4') represents `memcpy44`, an efficient memcpy
	// routine for word-aligned addresses.
	ldr r1, =0x3443
	// Call the lookup function
	blx r2

	// r3 = function pointer
	mov r3, r0

	ldr r0, =SRAM_BASE
	ldr r1, =(FLASH_BASE + 0x100) // skip this bootloader code
	ldr r2, =(SRAM_END - SRAM_BASE)
	blx r3

	pop {pc}
