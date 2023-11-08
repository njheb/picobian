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
#define SCB_BASE                        _BASE(0xe000ed00)
#define SCB_CPUID                       _REG(unsigned, 0xe000ed00)
#define SCB_ICSR                        _REG(unsigned, 0xe000ed04)
#define   SCB_ICSR_PENDSVSET __BIT(28)
    /* The Cortex-M0+ does not have the VECTACTIVE field. See:
     * https://forum.segger.com/index.php/Thread/4336-SOLVED-Cortex-M0-NoOS-Port-Read-Active-Interrupt-ID/
     */
#define SCB_SCR                         _REG(unsigned, 0xe000ed10)
#define SCB_SHPR                        _ARR(unsigned, 0xe000ed18)


/* Nested vectored interupt controller (Cortex-M0+) */
#define NVIC_BASE                       _BASE(0xe000e000)
#define NVIC_ISER                       _ARR(unsigned, 0xe000e100)
#define NVIC_ICER                       _ARR(unsigned, 0xe000e180)
#define NVIC_ISPR                       _ARR(unsigned, 0xe000e200)
#define NVIC_ICPR                       _ARR(unsigned, 0xe000e280)
#define NVIC_IPR                        _ARR(unsigned, 0xe000e400)


/* Single-cycle IO block (SIO) */
#define SIO_BASE                        _BASE(0xd0000000)
    /* Not to be confused with SCB.CPUID. CPU 0 reads 0, CPU 1 reads 1. */
#define SIO_CPUID                       _REG(unsigned, 0xd0000000)
#define SIO_GPIO_IN                     _REG(unsigned, 0xd0000004)
#define SIO_GPIO_HI_IN                  _REG(unsigned, 0xd0000008)
#define SIO_GPIO_OUT                    _REG(unsigned, 0xd0000010)
#define SIO_GPIO_OUT_SET                _REG(unsigned, 0xd0000014)
#define SIO_GPIO_OUT_CLR                _REG(unsigned, 0xd0000018)
#define SIO_GPIO_OUT_XOR                _REG(unsigned, 0xd000001c)
#define SIO_GPIO_OE                     _REG(unsigned, 0xd0000020)
#define SIO_GPIO_OE_SET                 _REG(unsigned, 0xd0000024)
#define SIO_GPIO_OE_CLR                 _REG(unsigned, 0xd0000028)
#define SIO_GPIO_OE_XOR                 _REG(unsigned, 0xd000002c)
#define SIO_GPIO_HI_OUT                 _REG(unsigned, 0xd0000030)
#define SIO_GPIO_HI_OUT_SET             _REG(unsigned, 0xd0000034)
#define SIO_GPIO_HI_OUT_CLR             _REG(unsigned, 0xd0000038)
#define SIO_GPIO_HI_OUR_XOR             _REG(unsigned, 0xd000003c)
#define SIO_GPIO_HI_OE                  _REG(unsigned, 0xd0000040)
#define SIO_GPIO_HI_OE_SET              _REG(unsigned, 0xd0000044)
#define SIO_GPIO_HI_OE_CLR              _REG(unsigned, 0xd0000048)
#define SIO_GPIO_HI_OE_XOR              _REG(unsigned, 0xd000004c)
    /* Further registers omitted */

/* 2.13.3 */
#define RESETS_BASE                     _BASE(0x4000c000)
#define RESETS_RESET                    _REG(unsigned, 0x4000c000)
#define RESETS_WDSEL                    _REG(unsigned, 0x4000c004)
#define RESETS_RESET_DONE               _REG(unsigned, 0x4000c008)
#define RESET_BIT_ADC   __BIT(0)
#define RESET_BIT_I2C0  __BIT(3)
#define RESET_BIT_I2C1  __BIT(4)
#define RESET_BIT_SPI0  __BIT(16)
#define RESET_BIT_SPI1  __BIT(17)
#define RESET_BIT_UART0 __BIT(22)
#define RESET_BIT_UART1 __BIT(23)

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
#define IO_BANK0_BASE                   _BASE(0x40014000)
#define IO_BANK0_GPIO0_STATUS           _REG(unsigned, 0x40014000)
#define IO_BANK0_GPIO0_CTRL             _REG(unsigned, 0x40014004)
#define IO_BANK0_GPIO1_STATUS           _REG(unsigned, 0x40014008)
#define IO_BANK0_GPIO1_CTRL             _REG(unsigned, 0x4001400c)
#define IO_BANK0_GPIO2_STATUS           _REG(unsigned, 0x40014010)
#define IO_BANK0_GPIO2_CTRL             _REG(unsigned, 0x40014014)
#define IO_BANK0_GPIO3_STATUS           _REG(unsigned, 0x40014018)
#define IO_BANK0_GPIO3_CTRL             _REG(unsigned, 0x4001401c)
#define IO_BANK0_GPIO4_STATUS           _REG(unsigned, 0x40014020)
#define IO_BANK0_GPIO4_CTRL             _REG(unsigned, 0x40014024)
#define IO_BANK0_GPIO5_STATUS           _REG(unsigned, 0x40014028)
#define IO_BANK0_GPIO5_CTRL             _REG(unsigned, 0x4001402c)
#define IO_BANK0_GPIO6_STATUS           _REG(unsigned, 0x40014030)
#define IO_BANK0_GPIO6_CTRL             _REG(unsigned, 0x40014034)
#define IO_BANK0_GPIO7_STATUS           _REG(unsigned, 0x40014038)
#define IO_BANK0_GPIO7_CTRL             _REG(unsigned, 0x4001403c)
#define IO_BANK0_GPIO8_STATUS           _REG(unsigned, 0x40014040)
#define IO_BANK0_GPIO8_CTRL             _REG(unsigned, 0x40014044)
#define IO_BANK0_GPIO9_STATUS           _REG(unsigned, 0x40014048)
#define IO_BANK0_GPIO9_CTRL             _REG(unsigned, 0x4001404c)
#define IO_BANK0_GPIO10_STATUS          _REG(unsigned, 0x40014050)
#define IO_BANK0_GPIO10_CTRL            _REG(unsigned, 0x40014054)
#define IO_BANK0_GPIO11_STATUS          _REG(unsigned, 0x40014058)
#define IO_BANK0_GPIO11_CTRL            _REG(unsigned, 0x4001405c)
#define IO_BANK0_GPIO12_STATUS          _REG(unsigned, 0x40014060)
#define IO_BANK0_GPIO12_CTRL            _REG(unsigned, 0x40014064)
#define IO_BANK0_GPIO13_STATUS          _REG(unsigned, 0x40014068)
#define IO_BANK0_GPIO13_CTRL            _REG(unsigned, 0x4001406c)
#define IO_BANK0_GPIO14_STATUS          _REG(unsigned, 0x40014070)
#define IO_BANK0_GPIO14_CTRL            _REG(unsigned, 0x40014074)
#define IO_BANK0_GPIO15_STATUS          _REG(unsigned, 0x40014078)
#define IO_BANK0_GPIO15_CTRL            _REG(unsigned, 0x4001407c)
#define IO_BANK0_GPIO16_STATUS          _REG(unsigned, 0x40014080)
#define IO_BANK0_GPIO16_CTRL            _REG(unsigned, 0x40014084)
#define IO_BANK0_GPIO17_STATUS          _REG(unsigned, 0x40014088)
#define IO_BANK0_GPIO17_CTRL            _REG(unsigned, 0x4001408c)
#define IO_BANK0_GPIO18_STATUS          _REG(unsigned, 0x40014090)
#define IO_BANK0_GPIO18_CTRL            _REG(unsigned, 0x40014094)
#define IO_BANK0_GPIO19_STATUS          _REG(unsigned, 0x40014098)
#define IO_BANK0_GPIO19_CTRL            _REG(unsigned, 0x4001409c)
#define IO_BANK0_GPIO20_STATUS          _REG(unsigned, 0x400140a0)
#define IO_BANK0_GPIO20_CTRL            _REG(unsigned, 0x400140a4)
#define IO_BANK0_GPIO21_STATUS          _REG(unsigned, 0x400140a8)
#define IO_BANK0_GPIO21_CTRL            _REG(unsigned, 0x400140ac)
#define IO_BANK0_GPIO22_STATUS          _REG(unsigned, 0x400140b0)
#define IO_BANK0_GPIO22_CTRL            _REG(unsigned, 0x400140b4)
#define IO_BANK0_GPIO23_STATUS          _REG(unsigned, 0x400140b8)
#define IO_BANK0_GPIO23_CTRL            _REG(unsigned, 0x400140bc)
#define IO_BANK0_GPIO24_STATUS          _REG(unsigned, 0x400140c0)
#define IO_BANK0_GPIO24_CTRL            _REG(unsigned, 0x400140c4)
#define IO_BANK0_GPIO25_STATUS          _REG(unsigned, 0x400140c8)
#define IO_BANK0_GPIO25_CTRL            _REG(unsigned, 0x400140cc)
#define IO_BANK0_GPIO26_STATUS          _REG(unsigned, 0x400140d0)
#define IO_BANK0_GPIO26_CTRL            _REG(unsigned, 0x400140d4)
#define IO_BANK0_GPIO27_STATUS          _REG(unsigned, 0x400140d8)
#define IO_BANK0_GPIO27_CTRL            _REG(unsigned, 0x400140dc)
#define IO_BANK0_GPIO28_STATUS          _REG(unsigned, 0x400140e0)
#define IO_BANK0_GPIO28_CTRL            _REG(unsigned, 0x400140e4)
#define IO_BANK0_GPIO29_STATUS          _REG(unsigned, 0x400140e8)
#define IO_BANK0_GPIO29_CTRL            _REG(unsigned, 0x400140ec)
#define IO_BANK0_INTR0                  _REG(unsigned, 0x400140f0)
#define IO_BANK0_INTR1                  _REG(unsigned, 0x400140f4)
#define IO_BANK0_INTR2                  _REG(unsigned, 0x400140f8)
#define IO_BANK0_INTR3                  _REG(unsigned, 0x400140fc)
#define IO_BANK0_PROC0_INTE0            _REG(unsigned, 0x40014100)
#define IO_BANK0_PROC0_INTE1            _REG(unsigned, 0x40014104)
#define IO_BANK0_PROC0_INTE2            _REG(unsigned, 0x40014108)
#define IO_BANK0_PROC0_INTE3            _REG(unsigned, 0x4001410c)
#define IO_BANK0_PROC0_INTF0            _REG(unsigned, 0x40014110)
#define IO_BANK0_PROC0_INTF1            _REG(unsigned, 0x40014114)
#define IO_BANK0_PROC0_INTF2            _REG(unsigned, 0x40014118)
#define IO_BANK0_PROC0_INTF3            _REG(unsigned, 0x4001411c)
#define IO_BANK0_PROC0_INTS0            _REG(unsigned, 0x40014120)
#define IO_BANK0_PROC0_INTS1            _REG(unsigned, 0x40014124)
#define IO_BANK0_PROC0_INTS2            _REG(unsigned, 0x40014128)
#define IO_BANK0_PROC0_INTS3            _REG(unsigned, 0x4001412c)
#define IO_BANK0_PROC1_INTE0            _REG(unsigned, 0x40014130)
#define IO_BANK0_PROC1_INTE1            _REG(unsigned, 0x40014134)
#define IO_BANK0_PROC1_INTE2            _REG(unsigned, 0x40014138)
#define IO_BANK0_PROC1_INTE3            _REG(unsigned, 0x4001413c)
#define IO_BANK0_PROC1_INTF0            _REG(unsigned, 0x40014140)
#define IO_BANK0_PROC1_INTF1            _REG(unsigned, 0x40014144)
#define IO_BANK0_PROC1_INTF2            _REG(unsigned, 0x40014148)
#define IO_BANK0_PROC1_INTF3            _REG(unsigned, 0x4001414c)
#define IO_BANK0_PROC1_INTS0            _REG(unsigned, 0x40014150)
#define IO_BANK0_PROC1_INTS1            _REG(unsigned, 0x40014154)
#define IO_BANK0_PROC1_INTS2            _REG(unsigned, 0x40014158)
#define IO_BANK0_PROC1_INTS3            _REG(unsigned, 0x4001415c)
#define IO_BANK0_DORMANT_WAKE_INTE0     _REG(unsigned, 0x40014160)
#define IO_BANK0_DORMANT_WAKE_INTE1     _REG(unsigned, 0x40014164)
#define IO_BANK0_DORMANT_WAKE_INTE2     _REG(unsigned, 0x40014168)
#define IO_BANK0_DORMANT_WAKE_INTE3     _REG(unsigned, 0x4001416c)
#define IO_BANK0_DORMANT_WAKE_INTF0     _REG(unsigned, 0x40014170)
#define IO_BANK0_DORMANT_WAKE_INTF1     _REG(unsigned, 0x40014174)
#define IO_BANK0_DORMANT_WAKE_INTF2     _REG(unsigned, 0x40014178)
#define IO_BANK0_DORMANT_WAKE_INTF3     _REG(unsigned, 0x4001417c)
#define IO_BANK0_DORMANT_WAKE_INTS0     _REG(unsigned, 0x40014180)
#define IO_BANK0_DORMANT_WAKE_INTS1     _REG(unsigned, 0x40014184)
#define IO_BANK0_DORMANT_WAKE_INTS2     _REG(unsigned, 0x40014188)
#define IO_BANK0_DORMANT_WAKE_INTS3     _REG(unsigned, 0x4001418c)

/* 2.18.6.2 */
#define IO_QSPI_BASE                    _BASE(0x40018000)
#define IO_QSPI_GPIO_QSPI_SCLK_STATUS   _REG(unsigned, 0x40018000)
#define IO_QSPI_GPIO_QSPI_SCLK_CTRL     _REG(unsigned, 0x40018004)
#define IO_QSPI_GPIO_QSPI_SS_STATUS     _REG(unsigned, 0x40018008)
#define IO_QSPI_GPIO_QSPI_SS_CTRL       _REG(unsigned, 0x4001800c)
#define IO_QSPI_GPIO_QSPI_SD0_STATUS    _REG(unsigned, 0x40018010)
#define IO_QSPI_GPIO_QSPI_SD0_CTRL      _REG(unsigned, 0x40018014)
#define IO_QSPI_GPIO_QSPI_SD1_STATUS    _REG(unsigned, 0x40018018)
#define IO_QSPI_GPIO_QSPI_SD1_CTRL      _REG(unsigned, 0x4001801c)
#define IO_QSPI_GPIO_QSPI_SD2_STATUS    _REG(unsigned, 0x40018020)
#define IO_QSPI_GPIO_QSPI_SD2_CTRL      _REG(unsigned, 0x40018024)
#define IO_QSPI_GPIO_QSPI_SD3_STATUS    _REG(unsigned, 0x40018028)
#define IO_QSPI_GPIO_QSPI_SD3_CTRL      _REG(unsigned, 0x4001802c)
#define IO_QSPI_INTR                    _REG(unsigned, 0x40018030)
#define IO_QSPI_PROC0_INTE              _REG(unsigned, 0x40018034)
#define IO_QSPI_PROC0_INTF              _REG(unsigned, 0x40018038)
#define IO_QSPI_PROC0_INTS              _REG(unsigned, 0x4001803c)
#define IO_QSPI_PROC1_INTE              _REG(unsigned, 0x40018040)
#define IO_QSPI_PROC1_INTF              _REG(unsigned, 0x40018044)
#define IO_QSPI_PROC1_INTS              _REG(unsigned, 0x40018048)
#define IO_QSPI_DORMANT_WAKE_INTE       _REG(unsigned, 0x4001804c)
#define IO_QSPI_DORMANT_WAKE_INTF       _REG(unsigned, 0x40018050)
#define IO_QSPI_DORMANT_WAKE_INTS       _REG(unsigned, 0x40018054)

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
#define PADS_BANK0_BASE                 _BASE(0x4001c000)
#define PADS_BANK0_VOLTAGE_SELECT       _REG(unsigned, 0x4001c000)
#define PADS_BANK0_GPIO0                _REG(unsigned, 0x4001c004)
#define PADS_BANK0_GPIO1                _REG(unsigned, 0x4001c008)
#define PADS_BANK0_GPIO2                _REG(unsigned, 0x4001c00c)
#define PADS_BANK0_GPIO3                _REG(unsigned, 0x4001c010)
#define PADS_BANK0_GPIO4                _REG(unsigned, 0x4001c014)
#define PADS_BANK0_GPIO5                _REG(unsigned, 0x4001c018)
#define PADS_BANK0_GPIO6                _REG(unsigned, 0x4001c01c)
#define PADS_BANK0_GPIO7                _REG(unsigned, 0x4001c020)
#define PADS_BANK0_GPIO8                _REG(unsigned, 0x4001c024)
#define PADS_BANK0_GPIO9                _REG(unsigned, 0x4001c028)
#define PADS_BANK0_GPIO10               _REG(unsigned, 0x4001c02c)
#define PADS_BANK0_GPIO11               _REG(unsigned, 0x4001c030)
#define PADS_BANK0_GPIO12               _REG(unsigned, 0x4001c034)
#define PADS_BANK0_GPIO13               _REG(unsigned, 0x4001c038)
#define PADS_BANK0_GPIO14               _REG(unsigned, 0x4001c03c)
#define PADS_BANK0_GPIO15               _REG(unsigned, 0x4001c040)
#define PADS_BANK0_GPIO16               _REG(unsigned, 0x4001c044)
#define PADS_BANK0_GPIO17               _REG(unsigned, 0x4001c048)
#define PADS_BANK0_GPIO18               _REG(unsigned, 0x4001c04c)
#define PADS_BANK0_GPIO19               _REG(unsigned, 0x4001c050)
#define PADS_BANK0_GPIO20               _REG(unsigned, 0x4001c054)
#define PADS_BANK0_GPIO21               _REG(unsigned, 0x4001c058)
#define PADS_BANK0_GPIO22               _REG(unsigned, 0x4001c05c)
#define PADS_BANK0_GPIO23               _REG(unsigned, 0x4001c060)
#define PADS_BANK0_GPIO24               _REG(unsigned, 0x4001c064)
#define PADS_BANK0_GPIO25               _REG(unsigned, 0x4001c068)
#define PADS_BANK0_GPIO26               _REG(unsigned, 0x4001c06c)
#define PADS_BANK0_GPIO27               _REG(unsigned, 0x4001c070)
#define PADS_BANK0_GPIO28               _REG(unsigned, 0x4001c074)
#define PADS_BANK0_GPIO29               _REG(unsigned, 0x4001c078)
#define PADS_BANK0_SWCLK                _REG(unsigned, 0x4001c07c)
#define PADS_BANK0_SWD                  _REG(unsigned, 0x4001c080)

/* Control of QSPI GPIO pads */
/* 2.16.6.4 */
#define PADS_QSPI_BASE                  _BASE(0x40020000)
#define PADS_QSPI_VOLTAGE_SELECT        _REG(unsigned, 0x40020000)
#define PADS_QSPI_GPIO_QSPI_SCLK        _REG(unsigned, 0x40020004)
#define PADS_QSPI_GPIO_QSPI_SD0         _REG(unsigned, 0x40020008)
#define PADS_QSPI_GPIO_QSPI_SD1         _REG(unsigned, 0x4002000c)
#define PADS_QSPI_GPIO_QSPI_SD2         _REG(unsigned, 0x40020010)
#define PADS_QSPI_GPIO_QSPI_SD3         _REG(unsigned, 0x40020014)
#define PADS_QSPI_GPIO_QSPI_SS          _REG(unsigned, 0x40020018)


/* 2.16.5 */
#define ROSC_BASE                       _BASE(0x40060000)
#define ROSC_CTRL                       _REG(unsigned, 0x40060000)
#define ROSC_CTRL_ENABLE __FIELD(12, 12)
#define ROSC_CTRL_ENABLE_DISABLE 0xd1e
#define ROSC_CTRL_ENABLE_ENABLE 0xfab
#define ROSC_FREQA                      _REG(unsigned, 0x40060004)
#define ROSC_FREQB                      _REG(unsigned, 0x40060008)
#define ROSC_DORMANT                    _REG(unsigned, 0x4006000c)
#define ROSC_DIV                        _REG(unsigned, 0x40060010)
#define ROSC_PHASE                      _REG(unsigned, 0x40060014)
#define ROSC_STATUS                     _REG(unsigned, 0x40060018)
#define ROSC_RANDOMBIT                  _REG(unsigned, 0x4006001c)
#define ROSC_COUNT                      _REG(unsigned, 0x40060020)


/* 2.15.7 */
#define XOSC_BASE                       _BASE(0x40024000)
#define XOSC_CTRL                       _REG(unsigned, 0x40024000)
#define XOSC_STATUS                     _REG(unsigned, 0x40024004)
#define XOSC_DORMANT                    _REG(unsigned, 0x40024008)
#define XOSC_STARTUP                    _REG(unsigned, 0x4002400c)
#define XOSC_DIV2                       _REG(unsigned, 0x40024010)
#define XOSC_PADREFCLK                  _REG(unsigned, 0x40024014)
#define XOSC_CLKSRC                     _REG(unsigned, 0x40024018)
#define XOSC_COUNT                      _REG(unsigned, 0x4002401c)


/* 2.17.4 */
#define PLL_SYS_BASE                    _BASE(0x40028000)
#define PLL_SYS_CS                      _REG(unsigned, 0x40028000)
#define PLL_SYS_PWR                     _REG(unsigned, 0x40028004)
#define PLL_SYS_FBDIV_INT               _REG(unsigned, 0x40028008)
#define PLL_SYS_PRIM                    _REG(unsigned, 0x4002800c)
#define PLL_USB_BASE                    _BASE(0x4002c000)
#define PLL_USB_CS                      _REG(unsigned, 0x4002c000)
#define PLL_USB_PWR                     _REG(unsigned, 0x4002c004)
#define PLL_USB_FBDIV_INT               _REG(unsigned, 0x4002c008)
#define PLL_USB_PRIM                    _REG(unsigned, 0x4002c00c)


/* 2.14.7 */
#define CLOCKS_BASE                     _BASE(0x40008000)
#define CLOCKS_CLK_GPOUT0_CTRL          _REG(unsigned, 0x40008000)
#define CLOCKS_CLK_GPOUT0_DIV           _REG(unsigned, 0x40008004)
#define CLOCKS_CLK_GPOUT0_SELECTED      _REG(unsigned, 0x40008008)
#define CLOCKS_CLK_GPOUT1_CTRL          _REG(unsigned, 0x4000800c)
#define CLOCKS_CLK_GPOUT1_DIV           _REG(unsigned, 0x40008010)
#define CLOCKS_CLK_GPOUT1_SELECTED      _REG(unsigned, 0x40008014)
#define CLOCKS_CLK_GPOUT2_CTRL          _REG(unsigned, 0x40008018)
#define CLOCKS_CLK_GPOUT2_DIV           _REG(unsigned, 0x4000801c)
#define CLOCKS_CLK_GPOUT2_SELECTED      _REG(unsigned, 0x40008020)
#define CLOCKS_CLK_GPOUT3_CTRL          _REG(unsigned, 0x40008024)
#define CLOCKS_CLK_GPOUT3_DIV           _REG(unsigned, 0x40008028)
#define CLOCKS_CLK_GPOUT3_SELECTED      _REG(unsigned, 0x4000802c)
#define CLOCKS_CLK_REF_CTRL             _REG(unsigned, 0x40008030)
#define CLOCKS_CLK_REF_DIV              _REG(unsigned, 0x40008034)
#define CLOCKS_CLK_REF_SELECTED         _REG(unsigned, 0x40008038)
#define CLOCKS_CLK_SYS_CTRL             _REG(unsigned, 0x4000803c)
#define CLOCKS_CLK_SYS_DIV              _REG(unsigned, 0x40008040)
#define CLOCKS_CLK_SYS_SELECTED         _REG(unsigned, 0x40008044)
#define CLOCKS_CLK_PERI_CTRL            _REG(unsigned, 0x40008048)
#define CLOCKS_CLK_PERI_SELECTED        _REG(unsigned, 0x40008050)
#define CLOCKS_CLK_USB_CTRL             _REG(unsigned, 0x40008054)
#define CLOCKS_CLK_USB_DIV              _REG(unsigned, 0x40008058)
#define CLOCKS_CLK_USB_SELECTED         _REG(unsigned, 0x4000805c)
#define CLOCKS_CLK_ADC_CTRL             _REG(unsigned, 0x40008060)
#define CLOCKS_CLK_ADC_DIV              _REG(unsigned, 0x40008064)
#define CLOCKS_CLK_ADC_SELECTED         _REG(unsigned, 0x40008068)
#define CLOCKS_CLK_RTC_CTRL             _REG(unsigned, 0x4000806c)
#define CLOCKS_CLK_RTC_DIV              _REG(unsigned, 0x40008070)
#define CLOCKS_CLK_RTC_SELECTED         _REG(unsigned, 0x40008074)
#define CLOCKS_CLK_SYS_RESUS_CTRL       _REG(unsigned, 0x40008078)
#define CLOCKS_CLK_SYS_RESUS_STATUS     _REG(unsigned, 0x4000807c)
#define CLOCKS_FC0_REF_KHZ              _REG(unsigned, 0x40008080)
#define CLOCKS_FC0_MIN_KHZ              _REG(unsigned, 0x40008084)
#define CLOCKS_FC0_MAX_KHZ              _REG(unsigned, 0x40008088)
#define CLOCKS_FC0_DELAY                _REG(unsigned, 0x4000808c)
#define CLOCKS_FC0_INTERVAL             _REG(unsigned, 0x40008090)
#define CLOCKS_FC0_SRC                  _REG(unsigned, 0x40008094)
#define CLOCKS_FC0_STATUS               _REG(unsigned, 0x40008098)
#define CLOCKS_FC0_RESULT               _REG(unsigned, 0x4000809c)
#define CLOCKS_WAKE_EN0                 _REG(unsigned, 0x400080a0)
#define CLOCKS_WAKE_EN1                 _REG(unsigned, 0x400080a4)
#define CLOCKS_SLEEP_EN0                _REG(unsigned, 0x400080a8)
#define CLOCKS_SLEEP_EN1                _REG(unsigned, 0x400080ac)
#define CLOCKS_ENABLED0                 _REG(unsigned, 0x400080b0)
#define CLOCKS_ENABLED1                 _REG(unsigned, 0x400080b4)
#define CLOCKS_INTR                     _REG(unsigned, 0x400080b8)
#define CLOCKS_INTE                     _REG(unsigned, 0x400080bc)
#define CLOCKS_INTF                     _REG(unsigned, 0x400080c0)
#define CLOCKS_INTS                     _REG(unsigned, 0x400080c4)


/* 4.7.5 */
#define TIMER_BASE                      _BASE(0x40054000)
    /* Since the timer value is 64-bit, it is accessed via a latched
     * pair of registers to prevent races. Always access low before
     * high! */
#define TIMER_TIMEHW                    _REG(unsigned, 0x40054000)
#define TIMER_TIMELW                    _REG(unsigned, 0x40054004)
#define TIMER_TIMEHR                    _REG(unsigned, 0x40054008)
#define TIMER_TIMELR                    _REG(unsigned, 0x4005400c)
#define TIMER_ALARM0                    _REG(unsigned, 0x40054010)
#define TIMER_ALARM1                    _REG(unsigned, 0x40054014)
#define TIMER_ALARM2                    _REG(unsigned, 0x40054018)
#define TIMER_ALARM3                    _REG(unsigned, 0x4005401c)
#define TIMER_ARMED                     _REG(unsigned, 0x40054020)
#define TIMER_TIMERAWH                  _REG(unsigned, 0x40054024)
#define TIMER_TIMERAWL                  _REG(unsigned, 0x40054028)
#define TIMER_DBGPAUSE                  _REG(unsigned, 0x4005402c)
#define TIMER_PAUSE                     _REG(unsigned, 0x40054030)
#define TIMER_INTR                      _REG(unsigned, 0x40054034)
#define TIMER_INTE                      _REG(unsigned, 0x40054038)
#define TIMER_INTF                      _REG(unsigned, 0x4005403c)
#define TIMER_INTS                      _REG(unsigned, 0x40054040)


/* 4.4.16 */
#define I2C0_BASE                       _BASE(0x40044000)
#define I2C0_CON                        _REG(unsigned, 0x40044000)
#define I2C0_TAR                        _REG(unsigned, 0x40044004)
#define I2C0_SAR                        _REG(unsigned, 0x40044008)
#define I2C0_DATA_CMD                   _REG(unsigned, 0x40044010)
#define I2C0_SS_SCL_HCNT                _REG(unsigned, 0x40044014)
#define I2C0_SS_SCL_LCNT                _REG(unsigned, 0x40044018)
#define I2C0_FS_SCL_HCNT                _REG(unsigned, 0x4004401c)
#define I2C0_FS_SCL_LCNT                _REG(unsigned, 0x40044020)
#define I2C0_INTR_STAT                  _REG(unsigned, 0x4004402c)
#define I2C0_INTR_MASK                  _REG(unsigned, 0x40044030)
#define I2C0_RAW_INTR_STAT              _REG(unsigned, 0x40044034)
#define I2C0_RX_TL                      _REG(unsigned, 0x40044038)
#define I2C0_TX_TL                      _REG(unsigned, 0x4004403c)
#define I2C0_CLR_INTR                   _REG(unsigned, 0x40044040)
#define I2C0_CLR_RX_UNDER               _REG(unsigned, 0x40044044)
#define I2C0_CLR_RX_OVER                _REG(unsigned, 0x40044048)
#define I2C0_CLR_TX_OVER                _REG(unsigned, 0x4004404c)
#define I2C0_CLR_RD_REQ                 _REG(unsigned, 0x40044050)
#define I2C0_CLR_TX_ABRT                _REG(unsigned, 0x40044054)
#define I2C0_CLR_RX_DONE                _REG(unsigned, 0x40044058)
#define I2C0_CLR_ACTIVITY               _REG(unsigned, 0x4004405c)
#define I2C0_CLR_STOP_DET               _REG(unsigned, 0x40044060)
#define I2C0_CLR_START_DET              _REG(unsigned, 0x40044064)
#define I2C0_CLR_GEN_CALL               _REG(unsigned, 0x40044068)
#define I2C0_ENABLE                     _REG(unsigned, 0x4004406c)
#define I2C0_STATUS                     _REG(unsigned, 0x40044070)
#define I2C0_TXFLR                      _REG(unsigned, 0x40044074)
#define I2C0_RXFLR                      _REG(unsigned, 0x40044078)
#define I2C0_SDA_HOLD                   _REG(unsigned, 0x4004407c)
#define I2C0_TX_ABRT_SOURCE             _REG(unsigned, 0x40044080)
#define I2C0_SLV_DATA_NACK_ONLY         _REG(unsigned, 0x40044084)
#define I2C0_DMA_CR                     _REG(unsigned, 0x40044088)
#define I2C0_DMA_TDLR                   _REG(unsigned, 0x4004408c)
#define I2C0_DMA_RDLR                   _REG(unsigned, 0x40044090)
#define I2C0_SDA_SETUP                  _REG(unsigned, 0x40044094)
#define I2C0_ACK_GENERAL_CALL           _REG(unsigned, 0x40044098)
#define I2C0_ENABLE_STATUS              _REG(unsigned, 0x4004409c)
#define I2C0_FS_SPKLEN                  _REG(unsigned, 0x400440a0)
#define I2C0_CLR_RESTART_DET            _REG(unsigned, 0x400440a8)
#define I2C0_COMP_PARAM_1               _REG(unsigned, 0x400440f4)
#define I2C0_COMP_VERSION               _REG(unsigned, 0x400440f8)
#define I2C0_COMP_TYPE                  _REG(unsigned, 0x400440fc)
#define I2C1_BASE                       _BASE(0x40048000)
#define I2C1_CON                        _REG(unsigned, 0x40048000)
#define I2C1_TAR                        _REG(unsigned, 0x40048004)
#define I2C1_SAR                        _REG(unsigned, 0x40048008)
#define I2C1_DATA_CMD                   _REG(unsigned, 0x40048010)
#define I2C1_SS_SCL_HCNT                _REG(unsigned, 0x40048014)
#define I2C1_SS_SCL_LCNT                _REG(unsigned, 0x40048018)
#define I2C1_FS_SCL_HCNT                _REG(unsigned, 0x4004801c)
#define I2C1_FS_SCL_LCNT                _REG(unsigned, 0x40048020)
#define I2C1_INTR_STAT                  _REG(unsigned, 0x4004802c)
#define I2C1_INTR_MASK                  _REG(unsigned, 0x40048030)
#define I2C1_RAW_INTR_STAT              _REG(unsigned, 0x40048034)
#define I2C1_RX_TL                      _REG(unsigned, 0x40048038)
#define I2C1_TX_TL                      _REG(unsigned, 0x4004803c)
#define I2C1_CLR_INTR                   _REG(unsigned, 0x40048040)
#define I2C1_CLR_RX_UNDER               _REG(unsigned, 0x40048044)
#define I2C1_CLR_RX_OVER                _REG(unsigned, 0x40048048)
#define I2C1_CLR_TX_OVER                _REG(unsigned, 0x4004804c)
#define I2C1_CLR_RD_REQ                 _REG(unsigned, 0x40048050)
#define I2C1_CLR_TX_ABRT                _REG(unsigned, 0x40048054)
#define I2C1_CLR_RX_DONE                _REG(unsigned, 0x40048058)
#define I2C1_CLR_ACTIVITY               _REG(unsigned, 0x4004805c)
#define I2C1_CLR_STOP_DET               _REG(unsigned, 0x40048060)
#define I2C1_CLR_START_DET              _REG(unsigned, 0x40048064)
#define I2C1_CLR_GEN_CALL               _REG(unsigned, 0x40048068)
#define I2C1_ENABLE                     _REG(unsigned, 0x4004806c)
#define I2C1_STATUS                     _REG(unsigned, 0x40048070)
#define I2C1_TXFLR                      _REG(unsigned, 0x40048074)
#define I2C1_RXFLR                      _REG(unsigned, 0x40048078)
#define I2C1_SDA_HOLD                   _REG(unsigned, 0x4004807c)
#define I2C1_TX_ABRT_SOURCE             _REG(unsigned, 0x40048080)
#define I2C1_SLV_DATA_NACK_ONLY         _REG(unsigned, 0x40048084)
#define I2C1_DMA_CR                     _REG(unsigned, 0x40048088)
#define I2C1_DMA_TDLR                   _REG(unsigned, 0x4004808c)
#define I2C1_DMA_RDLR                   _REG(unsigned, 0x40048090)
#define I2C1_SDA_SETUP                  _REG(unsigned, 0x40048094)
#define I2C1_ACK_GENERAL_CALL           _REG(unsigned, 0x40048098)
#define I2C1_ENABLE_STATUS              _REG(unsigned, 0x4004809c)
#define I2C1_FS_SPKLEN                  _REG(unsigned, 0x400480a0)
#define I2C1_CLR_RESTART_DET            _REG(unsigned, 0x400480a8)
#define I2C1_COMP_PARAM_1               _REG(unsigned, 0x400480f4)
#define I2C1_COMP_VERSION               _REG(unsigned, 0x400480f8)
#define I2C1_COMP_TYPE                  _REG(unsigned, 0x400480fc)


/* 4.3.8 */
#define UART0_BASE                      _BASE(0x40034000)
#define UART0_DR                        _REG(unsigned, 0x40034000)
#define UART0_RSR                       _REG(unsigned, 0x40034004)
#define UART0_FR                        _REG(unsigned, 0x40034018)
#define UART_FR_TXFF __BIT(5)
#define UART_FR_RXFF __BIT(6)
#define UART0_ILPR                      _REG(unsigned, 0x40034020)
#define UART0_IBRD                      _REG(unsigned, 0x40034024)
#define UART0_FBRD                      _REG(unsigned, 0x40034028)
#define UART0_LCR_H                     _REG(unsigned, 0x4003402c)
#define UART_LCR_H_BRK __BIT(0)
#define UART_LCR_H_PEN __BIT(1)
#define UART_LCR_H_EPS __BIT(2)
#define UART_LCR_H_STP2 __BIT(3)
#define UART_LCR_H_FEN __BIT(4)
#define UART_LCR_H_WLEN __FIELD(5, 2)
#define UART_LCR_H_SPS __BIT(7)
#define UART0_CR                        _REG(unsigned, 0x40034030)
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
#define UART0_IFLS                      _REG(unsigned, 0x40034034)
#define UART0_IMSC                      _REG(unsigned, 0x40034038)
#define UART0_RIS                       _REG(unsigned, 0x4003403c)
#define UART0_MIS                       _REG(unsigned, 0x40034040)
#define UART0_ICR                       _REG(unsigned, 0x40034044)
#define UART0_DMACR                     _REG(unsigned, 0x40034048)
#define UART0_PERIPHID0                 _REG(unsigned, 0x40034fe0)
#define UART0_PERIPHID1                 _REG(unsigned, 0x40034fe4)
#define UART0_PERIPHID2                 _REG(unsigned, 0x40034fe8)
#define UART0_PERIPHID3                 _REG(unsigned, 0x40034fec)
#define UART0_PCELLID0                  _REG(unsigned, 0x40034ff0)
#define UART0_PCELLID1                  _REG(unsigned, 0x40034ff4)
#define UART0_PCELLID2                  _REG(unsigned, 0x40034ff8)
#define UART0_PCELLID3                  _REG(unsigned, 0x40034ffc)
#define UART1_BASE                      _BASE(0x40038000)
#define UART1_DR                        _REG(unsigned, 0x40038000)
#define UART1_RSR                       _REG(unsigned, 0x40038004)
#define UART1_FR                        _REG(unsigned, 0x40038018)
#define UART1_ILPR                      _REG(unsigned, 0x40038020)
#define UART1_IBRD                      _REG(unsigned, 0x40038024)
#define UART1_FBRD                      _REG(unsigned, 0x40038028)
#define UART1_LCR_H                     _REG(unsigned, 0x4003802c)
#define UART1_CR                        _REG(unsigned, 0x40038030)
#define UART1_IFLS                      _REG(unsigned, 0x40038034)
#define UART1_IMSC                      _REG(unsigned, 0x40038038)
#define UART1_RIS                       _REG(unsigned, 0x4003803c)
#define UART1_MIS                       _REG(unsigned, 0x40038040)
#define UART1_ICR                       _REG(unsigned, 0x40038044)
#define UART1_DMACR                     _REG(unsigned, 0x40038048)
#define UART1_PERIPHID0                 _REG(unsigned, 0x40038fe0)
#define UART1_PERIPHID1                 _REG(unsigned, 0x40038fe4)
#define UART1_PERIPHID2                 _REG(unsigned, 0x40038fe8)
#define UART1_PERIPHID3                 _REG(unsigned, 0x40038fec)
#define UART1_PCELLID0                  _REG(unsigned, 0x40038ff0)
#define UART1_PCELLID1                  _REG(unsigned, 0x40038ff4)
#define UART1_PCELLID2                  _REG(unsigned, 0x40038ff8)
#define UART1_PCELLID3                  _REG(unsigned, 0x40038ffc)


/* 4.10.5 */
#define ADC_BASE                        _BASE(0x4004c000)
#define ADC_CS                          _REG(unsigned, 0x4004c000)
#define ADC_CS_EN __BIT(0)
#define ADC_CS_START_ONCE __BIT(2)
#define ADC_CS_READY __BIT(8)
#define ADC_CS_ERR __BIT(9)
#define ADC_RESULT                      _REG(unsigned, 0x4004c004)
#define ADC_FCS                         _REG(unsigned, 0x4004c008)
#define ADC_FIFO                        _REG(unsigned, 0x4004c00c)
#define ADC_DIV                         _REG(unsigned, 0x4004c010)
#define ADC_INTR                        _REG(unsigned, 0x4004c014)
#define ADC_INTE                        _REG(unsigned, 0x4004c018)
#define ADC_INTF                        _REG(unsigned, 0x4004c01c)
#define ADC_INTS                        _REG(unsigned, 0x4004c020)


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
