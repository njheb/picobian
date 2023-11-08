/* pi-pico/hardware.h */

#define PI_PICO 1

/* Hardware register definitions for RP2040 */
/* Cortex-M0+ documentation: https://developer.arm.com/documentation/dui0662/b?lang=en */
/* RP2040 documentation: https://files.seeedstudio.com/wiki/XIAO-RP2040/res/rp2040_datasheet.pdf */

#define BIT(i) (1 << (i))
#define GET_BIT(reg, n) (((reg) >> (n)) & 0x1)
#define SET_BIT(reg, n) reg |= BIT(n)
#define CLR_BIT(reg, n) reg &= ~BIT(n)

#define GET_BYTE(reg, n) (((reg) >> (8*(n))) & 0xff)
#define SET_BYTE(reg, n, v) \
    reg = (reg & ~(0xff << 8*n)) | ((v & 0xff) << 8*n)

/* The macros SET_FIELD, etc., are defined in an indirect way that
permits (because of the timing of CPP macro expansion) the 'field'
argument to be a macro that expands the a 'position, width' pair. */

#define SET_FIELD(reg, field, val) _SET_FIELD(reg, field, val)
#define _SET_FIELD(reg, pos, wd, val) \
    reg = (reg & ~_MASK(pos, wd)) | _FIELD(pos, wd, val)

#define GET_FIELD(reg, field) _GET_FIELD(reg, field)
#define _GET_FIELD(reg, pos, wd)  ((reg >> pos) & _MASK0(wd))

#define FIELD(field, val) _FIELD(field, val)
#define _FIELD(pos, wd, val)  (((val) & _MASK0(wd)) << pos)

#define MASK(field) _MASK(field)
#define _MASK(pos, wd)  (_MASK0(wd) << pos)

#define _MASK0(wd)  (~((-2) << (wd-1)))

#define __BIT(pos) pos
#define __FIELD(pos, wd) pos, wd

/* Device registers */
#define _BASE(addr) ((unsigned volatile *) addr)
#define _REG(ty, addr) (* (ty volatile *) addr)
#define _ARR(ty, addr) ((ty volatile *) addr)


#define GPIO_LED 25
/* USB (UART0) pins */
#define USB_TX 0
#define USB_RX 1

/* Exceptions */
#define SVC_IRQ    -5
#define PENDSV_IRQ -2
/* Interrupts (2.3.2) */
#define TIMER0_IRQ 0
#define TIMER1_IRQ 1
#define TIMER2_IRQ 2
#define TIMER3_IRQ 3
#define UART0_IRQ 20
#define UART1_IRQ 21
#define I2C0_IRQ 23
#define I2C1_IRQ 24
#define RTC_IRQ 25

#define N_INTERRUPTS 32


/* System contol block (Cortex-M0+) */
DEVICE scb {
    REGISTER unsigned CPUID @ 0x00;
    REGISTER unsigned ICSR @ 0x04;
#define   SCB_ICSR_PENDSVSET __BIT(28)
    /* The Cortex-M0+ does not have the VECTACTIVE field. See:
     * https://forum.segger.com/index.php/Thread/4336-SOLVED-Cortex-M0-NoOS-Port-Read-Active-Interrupt-ID/
     */
    REGISTER unsigned SCR @ 0x10;
    REGISTER unsigned SHPR[3] @ 0x18;
};
INSTANCE scb SCB @ 0xe000ed00;


/* Nested vectored interupt controller (Cortex-M0+) */
DEVICE nvic {
    REGISTER unsigned ISER[8] @ 0x100;
    REGISTER unsigned ICER[8] @ 0x180;
    REGISTER unsigned ISPR[8] @ 0x200;
    REGISTER unsigned ICPR[8] @ 0x280;
    REGISTER unsigned IPR[60] @ 0x400;
};
INSTANCE nvic NVIC @ 0xe000e000;


/* Single-cycle IO block (SIO) */
DEVICE sio {
    /* Not to be confused with SCB.CPUID. CPU 0 reads 0, CPU 1 reads 1. */
    REGISTER unsigned CPUID @ 0x00;
    REGISTER unsigned GPIO_IN @ 0x04;
    REGISTER unsigned GPIO_HI_IN @ 0x08;
    REGISTER unsigned GPIO_OUT @ 0x10;
    REGISTER unsigned GPIO_OUT_SET @ 0x14;
    REGISTER unsigned GPIO_OUT_CLR @ 0x18;
    REGISTER unsigned GPIO_OUT_XOR @ 0x1c;
    REGISTER unsigned GPIO_OE @ 0x20;
    REGISTER unsigned GPIO_OE_SET @ 0x24;
    REGISTER unsigned GPIO_OE_CLR @ 0x28;
    REGISTER unsigned GPIO_OE_XOR @ 0x2c;
    REGISTER unsigned GPIO_HI_OUT @ 0x30;
    REGISTER unsigned GPIO_HI_OUT_SET @ 0x34;
    REGISTER unsigned GPIO_HI_OUT_CLR @ 0x38;
    REGISTER unsigned GPIO_HI_OUR_XOR @ 0x3c;
    REGISTER unsigned GPIO_HI_OE @ 0x40;
    REGISTER unsigned GPIO_HI_OE_SET @ 0x44;
    REGISTER unsigned GPIO_HI_OE_CLR @ 0x48;
    REGISTER unsigned GPIO_HI_OE_XOR @ 0x4c;
    /* Further registers omitted */
};
INSTANCE sio SIO @ 0xd0000000;

/* 2.13.3 */
DEVICE resets {
    REGISTER unsigned RESET @ 0x00;
    REGISTER unsigned WDSEL @ 0x04;
    REGISTER unsigned RESET_DONE @ 0x08;
#define RESET_BIT_ADC   __BIT(0)
#define RESET_BIT_I2C0  __BIT(3)
#define RESET_BIT_I2C1  __BIT(4)
#define RESET_BIT_SPI0  __BIT(16)
#define RESET_BIT_SPI1  __BIT(17)
#define RESET_BIT_UART0 __BIT(22)
#define RESET_BIT_UART1 __BIT(23)
};
INSTANCE resets RESETS @ 0x4000c000;

/* Fields for IO_*_GPIO*_CTRL registers */
/* 2.18.6.1, 2.18.6.2 */
/* Function Select. Values are different per-pin. 31 (0x1f) is NULL. */
#define GPIO_CTRL_FUNCSEL __FIELD(0, 4)
#define GPIO_FUNC_XIP  0
#define GPIO_FUNC_SPI  1
#define GPIO_FUNC_UART 2
#define GPIO_FUNC_I2C  3
#define GPIO_FUNC_PWM  4
#define GPIO_FUNC_SIO  5
#define GPIO_FUNC_PIO0 6
#define GPIO_FUNC_PIO1 7
#define GPIO_FUNC_GPCK 8
#define GPIO_FUNC_USB  9
#define GPIO_FUNC_NULL 0x1f

/* 2.18.6.1 */
DEVICE io_user_bank {
    REGISTER unsigned GPIO0_STATUS @ 0x000;
    REGISTER unsigned GPIO0_CTRL   @ 0x004;
    REGISTER unsigned GPIO1_STATUS @ 0x008;
    REGISTER unsigned GPIO1_CTRL   @ 0x00c;
    REGISTER unsigned GPIO2_STATUS @ 0x010;
    REGISTER unsigned GPIO2_CTRL   @ 0x014;
    REGISTER unsigned GPIO3_STATUS @ 0x018;
    REGISTER unsigned GPIO3_CTRL   @ 0x01c;
    REGISTER unsigned GPIO4_STATUS @ 0x020;
    REGISTER unsigned GPIO4_CTRL   @ 0x024;
    REGISTER unsigned GPIO5_STATUS @ 0x028;
    REGISTER unsigned GPIO5_CTRL   @ 0x02c;
    REGISTER unsigned GPIO6_STATUS @ 0x030;
    REGISTER unsigned GPIO6_CTRL   @ 0x034;
    REGISTER unsigned GPIO7_STATUS @ 0x038;
    REGISTER unsigned GPIO7_CTRL   @ 0x03c;
    REGISTER unsigned GPIO8_STATUS @ 0x040;
    REGISTER unsigned GPIO8_CTRL   @ 0x044;
    REGISTER unsigned GPIO9_STATUS @ 0x048;
    REGISTER unsigned GPIO9_CTRL   @ 0x04c;
    REGISTER unsigned GPIO10_STATUS @ 0x050;
    REGISTER unsigned GPIO10_CTRL   @ 0x054;
    REGISTER unsigned GPIO11_STATUS @ 0x058;
    REGISTER unsigned GPIO11_CTRL   @ 0x05c;
    REGISTER unsigned GPIO12_STATUS @ 0x060;
    REGISTER unsigned GPIO12_CTRL   @ 0x064;
    REGISTER unsigned GPIO13_STATUS @ 0x068;
    REGISTER unsigned GPIO13_CTRL   @ 0x06c;
    REGISTER unsigned GPIO14_STATUS @ 0x070;
    REGISTER unsigned GPIO14_CTRL   @ 0x074;
    REGISTER unsigned GPIO15_STATUS @ 0x078;
    REGISTER unsigned GPIO15_CTRL   @ 0x07c;
    REGISTER unsigned GPIO16_STATUS @ 0x080;
    REGISTER unsigned GPIO16_CTRL   @ 0x084;
    REGISTER unsigned GPIO17_STATUS @ 0x088;
    REGISTER unsigned GPIO17_CTRL   @ 0x08c;
    REGISTER unsigned GPIO18_STATUS @ 0x090;
    REGISTER unsigned GPIO18_CTRL   @ 0x094;
    REGISTER unsigned GPIO19_STATUS @ 0x098;
    REGISTER unsigned GPIO19_CTRL   @ 0x09c;
    REGISTER unsigned GPIO20_STATUS @ 0x0a0;
    REGISTER unsigned GPIO20_CTRL   @ 0x0a4;
    REGISTER unsigned GPIO21_STATUS @ 0x0a8;
    REGISTER unsigned GPIO21_CTRL   @ 0x0ac;
    REGISTER unsigned GPIO22_STATUS @ 0x0b0;
    REGISTER unsigned GPIO22_CTRL   @ 0x0b4;
    REGISTER unsigned GPIO23_STATUS @ 0x0b8;
    REGISTER unsigned GPIO23_CTRL   @ 0x0bc;
    REGISTER unsigned GPIO24_STATUS @ 0x0c0;
    REGISTER unsigned GPIO24_CTRL   @ 0x0c4;
    REGISTER unsigned GPIO25_STATUS @ 0x0c8;
    REGISTER unsigned GPIO25_CTRL   @ 0x0cc;
    REGISTER unsigned GPIO26_STATUS @ 0x0d0;
    REGISTER unsigned GPIO26_CTRL   @ 0x0d4;
    REGISTER unsigned GPIO27_STATUS @ 0x0d8;
    REGISTER unsigned GPIO27_CTRL   @ 0x0dc;
    REGISTER unsigned GPIO28_STATUS @ 0x0e0;
    REGISTER unsigned GPIO28_CTRL   @ 0x0e4;
    REGISTER unsigned GPIO29_STATUS @ 0x0e8;
    REGISTER unsigned GPIO29_CTRL   @ 0x0ec;
    REGISTER unsigned INTR0 @ 0x0f0;
    REGISTER unsigned INTR1 @ 0x0f4;
    REGISTER unsigned INTR2 @ 0x0f8;
    REGISTER unsigned INTR3 @ 0x0fc;
    REGISTER unsigned PROC0_INTE0 @ 0x100;
    REGISTER unsigned PROC0_INTE1 @ 0x104;
    REGISTER unsigned PROC0_INTE2 @ 0x108;
    REGISTER unsigned PROC0_INTE3 @ 0x10c;
    REGISTER unsigned PROC0_INTF0 @ 0x110;
    REGISTER unsigned PROC0_INTF1 @ 0x114;
    REGISTER unsigned PROC0_INTF2 @ 0x118;
    REGISTER unsigned PROC0_INTF3 @ 0x11c;
    REGISTER unsigned PROC0_INTS0 @ 0x120;
    REGISTER unsigned PROC0_INTS1 @ 0x124;
    REGISTER unsigned PROC0_INTS2 @ 0x128;
    REGISTER unsigned PROC0_INTS3 @ 0x12c;
    REGISTER unsigned PROC1_INTE0 @ 0x130;
    REGISTER unsigned PROC1_INTE1 @ 0x134;
    REGISTER unsigned PROC1_INTE2 @ 0x138;
    REGISTER unsigned PROC1_INTE3 @ 0x13c;
    REGISTER unsigned PROC1_INTF0 @ 0x140;
    REGISTER unsigned PROC1_INTF1 @ 0x144;
    REGISTER unsigned PROC1_INTF2 @ 0x148;
    REGISTER unsigned PROC1_INTF3 @ 0x14c;
    REGISTER unsigned PROC1_INTS0 @ 0x150;
    REGISTER unsigned PROC1_INTS1 @ 0x154;
    REGISTER unsigned PROC1_INTS2 @ 0x158;
    REGISTER unsigned PROC1_INTS3 @ 0x15c;
    REGISTER unsigned DORMANT_WAKE_INTE0 @ 0x160;
    REGISTER unsigned DORMANT_WAKE_INTE1 @ 0x164;
    REGISTER unsigned DORMANT_WAKE_INTE2 @ 0x168;
    REGISTER unsigned DORMANT_WAKE_INTE3 @ 0x16c;
    REGISTER unsigned DORMANT_WAKE_INTF0 @ 0x170;
    REGISTER unsigned DORMANT_WAKE_INTF1 @ 0x174;
    REGISTER unsigned DORMANT_WAKE_INTF2 @ 0x178;
    REGISTER unsigned DORMANT_WAKE_INTF3 @ 0x17c;
    REGISTER unsigned DORMANT_WAKE_INTS0 @ 0x180;
    REGISTER unsigned DORMANT_WAKE_INTS1 @ 0x184;
    REGISTER unsigned DORMANT_WAKE_INTS2 @ 0x188;
    REGISTER unsigned DORMANT_WAKE_INTS3 @ 0x18c;
};
INSTANCE io_user_bank IO_BANK0 @ 0x40014000;

/* 2.18.6.2 */
DEVICE io_qspi_bank {
    REGISTER unsigned GPIO_QSPI_SCLK_STATUS @ 0x00;
    REGISTER unsigned GPIO_QSPI_SCLK_CTRL @ 0x04;
    REGISTER unsigned GPIO_QSPI_SS_STATUS @ 0x08;
    REGISTER unsigned GPIO_QSPI_SS_CTRL @ 0x0c;
    REGISTER unsigned GPIO_QSPI_SD0_STATUS @ 0x10;
    REGISTER unsigned GPIO_QSPI_SD0_CTRL   @ 0x14;
    REGISTER unsigned GPIO_QSPI_SD1_STATUS @ 0x18;
    REGISTER unsigned GPIO_QSPI_SD1_CTRL   @ 0x1c;
    REGISTER unsigned GPIO_QSPI_SD2_STATUS @ 0x20;
    REGISTER unsigned GPIO_QSPI_SD2_CTRL   @ 0x24;
    REGISTER unsigned GPIO_QSPI_SD3_STATUS @ 0x28;
    REGISTER unsigned GPIO_QSPI_SD3_CTRL   @ 0x2c;
    REGISTER unsigned INTR @ 0x30;
    REGISTER unsigned PROC0_INTE @ 0x34;
    REGISTER unsigned PROC0_INTF @ 0x38;
    REGISTER unsigned PROC0_INTS @ 0x3c;
    REGISTER unsigned PROC1_INTE @ 0x40;
    REGISTER unsigned PROC1_INTF @ 0x44;
    REGISTER unsigned PROC1_INTS @ 0x48;
    REGISTER unsigned DORMANT_WAKE_INTE @ 0x4c;
    REGISTER unsigned DORMANT_WAKE_INTF @ 0x50;
    REGISTER unsigned DORMANT_WAKE_INTS @ 0x54;
};
INSTANCE io_qspi_bank IO_QSPI @ 0x40018000;

/* Fields for PADS_*_GPIO* registers */
/* 2.18.6.3, 2.18.6.4 */
#define PADS_GPIO_OD      __BIT(7)      /* Output Disable */
#define PADS_GPIO_IE      __BIT(6)      /* Input Enable */
#define PADS_GPIO_DRIVE   __FIELD(4, 2) /* Drive Strength */
#define PADS_GPIO_PUE     __BIT(3)      /* Pull Up Enable */
#define PADS_GPIO_PDE     __BIT(2)      /* Pull Down Enable */
#define PADS_GPIO_SCHMITT __BIT(1)      /* Enable Schmitt Trigger */
#define PADS_GPIO_SLEW    __BIT(0)      /* Slew Rate Control */

#define GPIO_DRIVE_2MA 0
#define GPIO_DRIVE_4MA 1
#define GPIO_DRIVE_8MA 2
#define GPIO_DRIVE_12MA 3

/* Control of standard ("user") GPIO pads */
/* 2.16.6.3 */
DEVICE pads_user_bank {
    REGISTER unsigned VOLTAGE_SELECT @ 0x00;
    REGISTER unsigned GPIO0 @ 0x04;
    REGISTER unsigned GPIO1 @ 0x08;
    REGISTER unsigned GPIO2 @ 0x0c;
    REGISTER unsigned GPIO3 @ 0x10;
    REGISTER unsigned GPIO4 @ 0x14;
    REGISTER unsigned GPIO5 @ 0x18;
    REGISTER unsigned GPIO6 @ 0x1c;
    REGISTER unsigned GPIO7 @ 0x20;
    REGISTER unsigned GPIO8 @ 0x24;
    REGISTER unsigned GPIO9 @ 0x28;
    REGISTER unsigned GPIO10 @ 0x2c;
    REGISTER unsigned GPIO11 @ 0x30;
    REGISTER unsigned GPIO12 @ 0x34;
    REGISTER unsigned GPIO13 @ 0x38;
    REGISTER unsigned GPIO14 @ 0x3c;
    REGISTER unsigned GPIO15 @ 0x40;
    REGISTER unsigned GPIO16 @ 0x44;
    REGISTER unsigned GPIO17 @ 0x48;
    REGISTER unsigned GPIO18 @ 0x4c;
    REGISTER unsigned GPIO19 @ 0x50;
    REGISTER unsigned GPIO20 @ 0x54;
    REGISTER unsigned GPIO21 @ 0x58;
    REGISTER unsigned GPIO22 @ 0x5c;
    REGISTER unsigned GPIO23 @ 0x60;
    REGISTER unsigned GPIO24 @ 0x64;
    REGISTER unsigned GPIO25 @ 0x68;
    REGISTER unsigned GPIO26 @ 0x6c;
    REGISTER unsigned GPIO27 @ 0x70;
    REGISTER unsigned GPIO28 @ 0x74;
    REGISTER unsigned GPIO29 @ 0x78;
    REGISTER unsigned SWCLK @ 0x7c;
    REGISTER unsigned SWD @ 0x80;
};
INSTANCE pads_user_bank PADS_BANK0 @ 0x4001c000;

/* Control of QSPI GPIO pads */
/* 2.16.6.4 */
DEVICE pads_qspi_bank {
    REGISTER unsigned VOLTAGE_SELECT @ 0x00;
    REGISTER unsigned GPIO_QSPI_SCLK @ 0x04;
    REGISTER unsigned GPIO_QSPI_SD0 @ 0x08;
    REGISTER unsigned GPIO_QSPI_SD1 @ 0x0c;
    REGISTER unsigned GPIO_QSPI_SD2 @ 0x10;
    REGISTER unsigned GPIO_QSPI_SD3 @ 0x14;
    REGISTER unsigned GPIO_QSPI_SS @ 0x18;
};
INSTANCE pads_qspi_bank PADS_QSPI @ 0x40020000;


/* 2.16.5 */
DEVICE rosc {
    REGISTER unsigned CTRL @ 0x00;
#define ROSC_CTRL_ENABLE __FIELD(12, 12)
#define ROSC_CTRL_ENABLE_DISABLE 0xd1e
#define ROSC_CTRL_ENABLE_ENABLE 0xfab
    REGISTER unsigned FREQA @ 0x04;
    REGISTER unsigned FREQB @ 0x08;
    REGISTER unsigned DORMANT @ 0x0c;
    REGISTER unsigned DIV @ 0x10;
    REGISTER unsigned PHASE @ 0x14;
    REGISTER unsigned STATUS @ 0x18;
    REGISTER unsigned RANDOMBIT @ 0x1c;
    REGISTER unsigned COUNT @ 0x20;
};
INSTANCE rosc ROSC @ 0x40060000;


/* 2.15.7 */
DEVICE xosc {
    REGISTER unsigned CTRL @ 0x00;
    REGISTER unsigned STATUS @ 0x04;
    REGISTER unsigned DORMANT @ 0x08;
    REGISTER unsigned STARTUP @ 0x0c;
    REGISTER unsigned DIV2 @ 0x10;
    REGISTER unsigned PADREFCLK @ 0x14;
    REGISTER unsigned CLKSRC @ 0x18;
    REGISTER unsigned COUNT @ 0x1c;
};
INSTANCE xosc XOSC @ 0x40024000;


/* 2.17.4 */
DEVICE pll {
    REGISTER unsigned CS @ 0x00;
    REGISTER unsigned PWR @ 0x04;
    REGISTER unsigned FBDIV_INT @ 0x08;
    REGISTER unsigned PRIM @ 0x0c;
};
INSTANCE pll PLL_SYS @ 0x40028000;
INSTANCE pll PLL_USB @ 0x4002c000;


/* 2.14.7 */
DEVICE clocks {
    REGISTER unsigned CLK_GPOUT0_CTRL @ 0x00;
    REGISTER unsigned CLK_GPOUT0_DIV @ 0x04;
    REGISTER unsigned CLK_GPOUT0_SELECTED @ 0x08;
    REGISTER unsigned CLK_GPOUT1_CTRL @ 0x0c;
    REGISTER unsigned CLK_GPOUT1_DIV @ 0x10;
    REGISTER unsigned CLK_GPOUT1_SELECTED @ 0x14;
    REGISTER unsigned CLK_GPOUT2_CTRL @ 0x18;
    REGISTER unsigned CLK_GPOUT2_DIV @ 0x1c;
    REGISTER unsigned CLK_GPOUT2_SELECTED @ 0x20;
    REGISTER unsigned CLK_GPOUT3_CTRL @ 0x24;
    REGISTER unsigned CLK_GPOUT3_DIV @ 0x28;
    REGISTER unsigned CLK_GPOUT3_SELECTED @ 0x2c;
    REGISTER unsigned CLK_REF_CTRL @ 0x30;
    REGISTER unsigned CLK_REF_DIV @ 0x34;
    REGISTER unsigned CLK_REF_SELECTED @ 0x38;
    REGISTER unsigned CLK_SYS_CTRL @ 0x3c;
    REGISTER unsigned CLK_SYS_DIV @ 0x40;
    REGISTER unsigned CLK_SYS_SELECTED @ 0x44;
    REGISTER unsigned CLK_PERI_CTRL @ 0x48;
    REGISTER unsigned CLK_PERI_SELECTED @ 0x50;
    REGISTER unsigned CLK_USB_CTRL @ 0x54;
    REGISTER unsigned CLK_USB_DIV @ 0x58;
    REGISTER unsigned CLK_USB_SELECTED @ 0x5c;
    REGISTER unsigned CLK_ADC_CTRL @ 0x60;
    REGISTER unsigned CLK_ADC_DIV @ 0x64;
    REGISTER unsigned CLK_ADC_SELECTED @ 0x68;
    REGISTER unsigned CLK_RTC_CTRL @ 0x6c;
    REGISTER unsigned CLK_RTC_DIV @ 0x70;
    REGISTER unsigned CLK_RTC_SELECTED @ 0x74;
    REGISTER unsigned CLK_SYS_RESUS_CTRL @ 0x78;
    REGISTER unsigned CLK_SYS_RESUS_STATUS @ 0x7c;
    REGISTER unsigned FC0_REF_KHZ @ 0x80;
    REGISTER unsigned FC0_MIN_KHZ @ 0x84;
    REGISTER unsigned FC0_MAX_KHZ @ 0x88;
    REGISTER unsigned FC0_DELAY @ 0x8c;
    REGISTER unsigned FC0_INTERVAL @ 0x90;
    REGISTER unsigned FC0_SRC @ 0x94;
    REGISTER unsigned FC0_STATUS @ 0x98;
    REGISTER unsigned FC0_RESULT @ 0x9c;
    REGISTER unsigned WAKE_EN0 @ 0xa0;
    REGISTER unsigned WAKE_EN1 @ 0xa4;
    REGISTER unsigned SLEEP_EN0 @ 0xa8;
    REGISTER unsigned SLEEP_EN1 @ 0xac;
    REGISTER unsigned ENABLED0 @ 0xb0;
    REGISTER unsigned ENABLED1 @ 0xb4;
    REGISTER unsigned INTR @ 0xb8;
    REGISTER unsigned INTE @ 0xbc;
    REGISTER unsigned INTF @ 0xc0;
    REGISTER unsigned INTS @ 0xc4;
};
INSTANCE clocks CLOCKS @ 0x40008000;


/* 4.7.5 */
DEVICE timer {
    /* Since the timer value is 64-bit, it is accessed via a latched
     * pair of registers to prevent races. Always access low before
     * high! */
    REGISTER unsigned TIMEHW @ 0x00; /* Write high byte */
    REGISTER unsigned TIMELW @ 0x04; /* Write low byte  */
    REGISTER unsigned TIMEHR @ 0x08; /* Read high byte  */
    REGISTER unsigned TIMELR @ 0x0c; /* Read low byte   */
    REGISTER unsigned ALARM0 @ 0x10; /* Fire alarm 0 when TIMELR == this */
    REGISTER unsigned ALARM1 @ 0x14; /* Fire alarm 1 when TIMELR == this */
    REGISTER unsigned ALARM2 @ 0x18; /* Fire alarm 2 when TIMELR == this */
    REGISTER unsigned ALARM3 @ 0x1c; /* Fire alarm 3 when TIMELR == this */
    REGISTER unsigned ARMED @ 0x20;
    REGISTER unsigned TIMERAWH @ 0x24; /* Raw value (not latched). Use with caution. */
    REGISTER unsigned TIMERAWL @ 0x28; /* Raw value (not latched). Use with caution. */
    REGISTER unsigned DBGPAUSE @ 0x2c;
    REGISTER unsigned PAUSE @ 0x30;
    REGISTER unsigned INTR @ 0x34;
    REGISTER unsigned INTE @ 0x38;
    REGISTER unsigned INTF @ 0x3c;
    REGISTER unsigned INTS @ 0x40;
};
INSTANCE timer TIMER @ 0x40054000;


/* 4.4.16 */
DEVICE i2c {
    REGISTER unsigned CON @ 0x00;
    REGISTER unsigned TAR @ 0x04;
    REGISTER unsigned SAR @ 0x08;
    REGISTER unsigned DATA_CMD @ 0x10;
    REGISTER unsigned SS_SCL_HCNT @ 0x14;
    REGISTER unsigned SS_SCL_LCNT @ 0x18;
    REGISTER unsigned FS_SCL_HCNT @ 0x1c;
    REGISTER unsigned FS_SCL_LCNT @ 0x20;
    REGISTER unsigned INTR_STAT @ 0x2c;
    REGISTER unsigned INTR_MASK @ 0x30;
    REGISTER unsigned RAW_INTR_STAT @ 0x34;
    REGISTER unsigned RX_TL @ 0x38;
    REGISTER unsigned TX_TL @ 0x3c;
    REGISTER unsigned CLR_INTR @ 0x40;
    REGISTER unsigned CLR_RX_UNDER @ 0x44;
    REGISTER unsigned CLR_RX_OVER @ 0x48;
    REGISTER unsigned CLR_TX_OVER @ 0x4c;
    REGISTER unsigned CLR_RD_REQ @ 0x50;
    REGISTER unsigned CLR_TX_ABRT @ 0x54;
    REGISTER unsigned CLR_RX_DONE @ 0x58;
    REGISTER unsigned CLR_ACTIVITY @ 0x5c;
    REGISTER unsigned CLR_STOP_DET @ 0x60;
    REGISTER unsigned CLR_START_DET @ 0x64;
    REGISTER unsigned CLR_GEN_CALL @ 0x68;
    REGISTER unsigned ENABLE @ 0x6c;
    REGISTER unsigned STATUS @ 0x70;
    REGISTER unsigned TXFLR @ 0x74;
    REGISTER unsigned RXFLR @ 0x78;
    REGISTER unsigned SDA_HOLD @ 0x7c;
    REGISTER unsigned TX_ABRT_SOURCE @ 0x80;
    REGISTER unsigned SLV_DATA_NACK_ONLY @ 0x84;
    REGISTER unsigned DMA_CR @ 0x88;
    REGISTER unsigned DMA_TDLR @ 0x8c;
    REGISTER unsigned DMA_RDLR @ 0x90;
    REGISTER unsigned SDA_SETUP @ 0x94;
    REGISTER unsigned ACK_GENERAL_CALL @ 0x98;
    REGISTER unsigned ENABLE_STATUS @ 0x9c;
    REGISTER unsigned FS_SPKLEN @ 0xa0;
    REGISTER unsigned CLR_RESTART_DET @ 0xa8;
    REGISTER unsigned COMP_PARAM_1 @ 0xf4;
    REGISTER unsigned COMP_VERSION @ 0xf8;
    REGISTER unsigned COMP_TYPE @ 0xfc;
};
INSTANCE i2c I2C0 @ 0x40044000;
INSTANCE i2c I2C1 @ 0x40048000;


/* 4.3.8 */
DEVICE uart {
    REGISTER unsigned DR @ 0x000;
    REGISTER unsigned RSR @ 0x004;
    REGISTER unsigned FR @ 0x018;
#define UART_FR_TXFF __BIT(5)
#define UART_FR_RXFF __BIT(6)
    REGISTER unsigned ILPR @ 0x020;
    REGISTER unsigned IBRD @ 0x024;
    REGISTER unsigned FBRD @ 0x028;
    REGISTER unsigned LCR_H @ 0x02c;
#define UART_LCR_H_BRK __BIT(0)
#define UART_LCR_H_PEN __BIT(1)
#define UART_LCR_H_EPS __BIT(2)
#define UART_LCR_H_STP2 __BIT(3)
#define UART_LCR_H_FEN __BIT(4)
#define UART_LCR_H_WLEN __FIELD(5, 2)
#define UART_LCR_H_SPS __BIT(7)
    REGISTER unsigned CR @ 0x030;
#define UART_CR_UARTEN __BIT(0)
#define UART_CR_SIREN  __BIT(1)
#define UART_CR_SIRLP  __BIT(2)
#define UART_CR_LBE    __BIT(7)
#define UART_CR_TXE    __BIT(8)
#define UART_CR_RXE    __BIT(9)
#define UART_CR_DTR    __BIT(10)
#define UART_CR_RTS    __BIT(11)
#define UART_CR_OUT1   __BIT(12)
#define UART_CR_OUT2   __BIT(13)
#define UART_CR_RTSEN  __BIT(14)
#define UART_CR_CTSEN  __BIT(15)
    REGISTER unsigned IFLS @ 0x034;
    REGISTER unsigned IMSC @ 0x038;
    REGISTER unsigned RIS @ 0x03c;
    REGISTER unsigned MIS @ 0x040;
    REGISTER unsigned ICR @ 0x044;
    REGISTER unsigned DMACR @ 0x048;
    REGISTER unsigned PERIPHID0 @ 0xfe0;
    REGISTER unsigned PERIPHID1 @ 0xfe4;
    REGISTER unsigned PERIPHID2 @ 0xfe8;
    REGISTER unsigned PERIPHID3 @ 0xfec;
    REGISTER unsigned PCELLID0 @ 0xff0;
    REGISTER unsigned PCELLID1 @ 0xff4;
    REGISTER unsigned PCELLID2 @ 0xff8;
    REGISTER unsigned PCELLID3 @ 0xffc;
};
INSTANCE uart UART0 @ 0x40034000;
INSTANCE uart UART1 @ 0x40038000;


/* 4.10.5 */
DEVICE adc {
    REGISTER unsigned CS @ 0x00;
#define ADC_CS_EN __BIT(0)
#define ADC_CS_START_ONCE __BIT(2)
#define ADC_CS_READY __BIT(8)
#define ADC_CS_ERR __BIT(9)
    REGISTER unsigned RESULT @ 0x04; /* Mask low 12 bits for result */
    REGISTER unsigned FCS @ 0x08;
    REGISTER unsigned FIFO @ 0x0c;
    REGISTER unsigned DIV @ 0x10;
    REGISTER unsigned INTR @ 0x14;
    REGISTER unsigned INTE @ 0x18;
    REGISTER unsigned INTF @ 0x1c;
    REGISTER unsigned INTS @ 0x20;
};
INSTANCE adc ADC @ 0x4004c000;


/* NVIC stuff */

/* irq_priority -- set priority of an IRQ from 0 (highest) to 255 */
void irq_priority(int irq, unsigned priority);

/* enable_irq -- enable interrupts from an IRQ */
#define enable_irq(irq)  NVIC_ISER[0] = BIT(irq)

/* disable_irq -- disable interrupts from a specific IRQ */
#define disable_irq(irq)  NVIC_ICER[0] = BIT(irq)

/* clear_pending -- clear pending interrupt from an IRQ */
#define clear_pending(irq)  NVIC_ICPR[0] = BIT(irq)

/* reschedule -- request PendSV interrupt */
#define reschedule()  SCB_ICSR = BIT(SCB_ICSR_PENDSVSET)

/* active_irq -- find active interrupt: returns -16 to 31 */
#define active_irq()  ({ unsigned x; asm volatile ("mrs %0, ipsr" : "=r" (x)); (x & 0x3f) - 16; })

/* delay_loop -- timed delay */
void delay_loop(unsigned usec);


/* GPIO convenience */

#ifndef INLINE
#define INLINE inline
#endif

/* gpio_dir -- set GPIO direction */
/* 0 = input, 1 = output */
INLINE void gpio_dir(unsigned pin, unsigned dir) {
    if (dir) {
        SIO_GPIO_OE_SET = BIT(pin);
    } else {
        SIO_GPIO_OE_CLR = BIT(pin);
    }
}

/* gpio_connect -- connect pin for input */
INLINE void gpio_connect(unsigned pin) {
    volatile unsigned *gpio_reg = &PADS_BANK0_GPIO0 + pin;
    SET_BIT(*gpio_reg, PADS_GPIO_IE);
}

/* gpio_drive -- set GPIO drive strength */
INLINE void gpio_drive(unsigned pin, unsigned mode) {
    volatile unsigned *gpio_reg = &PADS_BANK0_GPIO0 + pin;
    SET_FIELD(*gpio_reg, PADS_GPIO_DRIVE, mode);
}

/* gpio_out -- set GPIO output value */
INLINE void gpio_out(unsigned pin, unsigned value) {
    if (value) {
        SIO_GPIO_OUT_SET = BIT(pin);
    } else {
        SIO_GPIO_OUT_CLR = BIT(pin);
    }
}

/* gpio_in -- get GPIO input bit */
INLINE unsigned gpio_in(unsigned pin) {
    return GET_BIT(SIO_GPIO_IN, pin);
}

/* gpio_set_func -- set function of GPIO pin */
INLINE void gpio_set_func(unsigned pin, unsigned func) {
    volatile unsigned *gpio_ctrl = &IO_BANK0_GPIO0_CTRL + (2 * pin); /* skip STATUS registers */
    SET_FIELD(*gpio_ctrl, GPIO_CTRL_FUNCSEL, func);
}

/* Other convenience */

INLINE void reset_subsystem(unsigned bit) {
    SET_BIT(RESETS_RESET, bit);
    CLR_BIT(RESETS_RESET, bit);
    while (!GET_BIT(RESETS_RESET_DONE, bit));
}


/* A few assembler macros for single instructions. */
#define pause()         asm volatile ("wfe")
#define intr_disable()  asm volatile ("cpsid i")
#define intr_enable()   asm volatile ("cpsie i")
#define get_primask()   ({ unsigned x;                                   \
                           asm volatile ("mrs %0, primask" : "=r" (x)); x; })
#define set_primask(x)  asm volatile ("msr primask, %0" : : "r" (x))
#define nop()           asm volatile ("nop")

/* The rate of the crystal oscillator attached to the system (12MHz) */
#define XOSC_HZ 12000000

/* The startup.c config will configure PLL SYS appropriately to give a
 * resulting rate of SYS_CLK_HZ. The clocks CLK_SYS and CLK_PERI are
 * configured to also match this rate. */
#define SYS_CLK_HZ 125000000
