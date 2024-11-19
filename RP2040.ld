/* RP2040.ld */

MEMORY {
    /* RAM includes SRAM_STRIPED and SRAM4. SRAM5 is not
		 * included as it contains the stage2 bootloader. */
    RAM (rwx) : ORIGIN = 0x20000000, LENGTH = 260K
}

OUTPUT_FORMAT ("elf32-littlearm")

/* Setting the entry address helps GDB to find the stack bottom */
ENTRY(__reset)

SECTIONS {
    .text : {
        KEEP(*(.vectors))
        *(.text*)
        *(.rodata*)
        . = ALIGN(4);
        __etext = .;

        PROVIDE(nmi_handler = default_handler);
        PROVIDE(hardfault_handler = default_handler);
        PROVIDE(svc_handler = default_handler);
        PROVIDE(pendsv_handler = default_handler);
        PROVIDE(systick_handler = default_handler);
        PROVIDE(timer0_handler = default_handler);
        PROVIDE(timer1_handler = default_handler);
        PROVIDE(timer2_handler = default_handler);
        PROVIDE(timer3_handler = default_handler);
        PROVIDE(uart0_handler = default_handler);
        PROVIDE(uart1_handler = default_handler);
        PROVIDE(adc_handler = default_handler);
        PROVIDE(i2c0_handler = default_handler);
        PROVIDE(i2c1_handler = default_handler);
        PROVIDE(rtc_handler = default_handler);
    } > RAM

    /DISCARD/ : {
        /* Some parts of the GCC library have C++ exception data,
           which is of no use to us. */
        *(.ARM.extab)
        *(.ARM.exidx)
    }

    .data : AT (__etext) {
        __data_start = .;
        *(.data*)
        . = ALIGN(4);
        __data_end = .;
    } > RAM

    .bss : {
        __bss_start = .;
        *(.bss*)
        *(COMMON)
        . = ALIGN(4);
        __bss_end = .;
        __end = .;
    } > RAM

    /* Set stack top to end of RAM, and move stack limit down by
       size of stack */
    __stack = ORIGIN(RAM) + LENGTH(RAM);
    __stack_limit = __stack - 2048;

    /* Check if data + stack exceeds RAM limit */
    ASSERT(__stack_limit >= __end, "region RAM overlaps with stack")
}
