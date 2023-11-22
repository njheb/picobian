/* common/startup.c */
/* Copyright (c) 2018 J. M. Spivey */

#define INLINE                  /* Create actual copies of inline functions */
#include "hardware.h"

/* init -- main program, creates application processes */
void init(void);

void default_start(void)
{
    init();                    /* Call the main program. */
    while (1) pause();         /* Halt if init() returns */
}

void __start(void) __attribute((weak, alias("default_start")));

/* The next four routines can be used in C compiler output, even if
not mentioned in the source. */

/* memcpy -- copy n bytes from src to dest (non-overlapping) */
void *memcpy(void *dest, const void *src, unsigned n)
{
    unsigned char *p = dest;
    const unsigned char *q = src;
    while (n-- > 0) *p++ = *q++;
    return dest;
}

/* memmove -- copy n bytes from src to dest, allowing overlaps */
void *memmove(void *dest, const void *src, unsigned n)
{
    unsigned char *p = dest;
    const unsigned char *q = src;
    if (dest <= src)
        while (n-- > 0) *p++ = *q++;
    else {
        p += n; q += n;
        while (n-- > 0) *--p = *--q;
    }
    return dest;
}
    
/* memset -- set n bytes of dest to byte x */
void *memset(void *dest, unsigned x, unsigned n)
{
    unsigned char *p = dest;
    while (n-- > 0) *p++ = x;
    return dest;
}

/* memcmp -- compare n bytes */
int memcmp(const void *pp, const void *qq, int n)
{
    const unsigned char *p = pp, *q = qq;
    while (n-- > 0) {
        if (*p++ != *q++)
            return (p[-1] < q[-1] ? -1 : 1);
    }
    return 0;
}

static unsigned char __stack_core1_limit[2048];

static void __reset_core1(void) {
    gpio_set_func(GPIO_LED, GPIO_FUNC_SIO);
    gpio_dir(GPIO_LED, 1);
    unsigned x = 1;
    for (;;) {
        gpio_out(GPIO_LED, x);
        unsigned wait_for = TIMER_TIMERAWL + 1000000; // 1000ms
        while (TIMER_TIMERAWL < wait_for);
        x = !x;
    }
}

static void init_core1(void) {
    /* Logic from RP2040 datasheet section 2.8.2 and the Pico SDK */
    extern void *__vectors[];
    unsigned char __stack_core1 = (unsigned)__stack_core1_limit + sizeof __stack_core1_limit;
    const unsigned cmd_sequence[] = { 0, 0, 1, (unsigned)__vectors, (unsigned)__stack_core1, (unsigned)__reset_core1 };
    unsigned seq = 0;
    while (seq < sizeof cmd_sequence / sizeof cmd_sequence[0]) {
        unsigned cmd = cmd_sequence[seq];
        /* If the command is 0, flush the FIFO completely */
        if (!cmd) {
            while (GET_BIT(SIO_FIFO_ST, SIO_FIFO_ST_VLD)) {
                (void)SIO_FIFO_RD;
            }
            asm volatile ("sev");
        }
        /* Send the command to the other core */
        while (!GET_BIT(SIO_FIFO_ST, SIO_FIFO_ST_RDY));
        SIO_FIFO_WR = cmd;
        asm volatile ("sev");
        /* Move to next state on correct response, otherwise start over */
        while (!GET_BIT(SIO_FIFO_ST, SIO_FIFO_ST_VLD)) {
            asm volatile ("wfe");
        }
        seq = SIO_FIFO_RD == cmd ? seq + 1 : 0;
    }
}

/* Addresses set by the linker */
extern unsigned char __bss_start[], __bss_end[], __stack[];

/* __reset -- the system starts here */
void __reset(void)
{
    int bss_size = __bss_end - __bss_start;
    memset(__bss_start, 0, bss_size);

#define WANT_RESET (BIT(RESET_IO_BANK0) | BIT(RESET_PADS_BANK0) | BIT(RESET_PLL_SYS) | BIT(RESET_TIMER))
    RESETS_RESET = 0xffffffff;
    RESETS_RESET &= ~(unsigned)WANT_RESET;
    while (~RESETS_RESET_DONE & WANT_RESET);
#undef WANT_RESET

    /* The timer requires the watchdog tick to be running */
    SET_FIELD(WATCHDOG_TICK, WATCHDOG_TICK_CYCLES, XOSC_HZ / 1000000);
    SET_BIT(WATCHDOG_TICK, WATCHDOG_TICK_ENABLE);

    /* Enable the crystal oscillator (XOSC) */
    _Static_assert(XOSC_HZ == 12000000);
    XOSC_STARTUP = 48; /* sufficient startup delay */
    XOSC_CTRL = 0xaa0 | (0xfab << 12); /* frequency range 1-15MHz, enable */
    while (!(XOSC_STATUS >> 31)) {
      /* spin until XOSC is stable */
    }

    /* Configure PLL SYS to be driven by XOSC at 125MHz */
    _Static_assert(XOSC_HZ == 12000000 && SYS_CLK_HZ == 125000000);
    PLL_SYS_PWR = 0xffffffff; /* disable PLL SYS */
    PLL_SYS_CS = 1; /* REFDIV = 1, BYPASS = 0 */
    PLL_SYS_FBDIV_INT = 125;
    PLL_SYS_PRIM = (2 << 16) | (6 << 12); /* post-divider = 2*6 = 12 */
    PLL_SYS_PWR = 0; /* enable PLL SYS */

    /* Drive CLK_REF by XOSC and CLK_SYS by PLL SYS. That way, we can also
     * disable the on-chip ring oscillator (ROSC) to save power. */
    CLOCKS_CLK_REF_CTRL = 2; /* CLK_REF SRC = xosc_clksrc */
    /* We want to drive CLK_SYS from its aux src. This can glitch when changing,
     * so you need to set it before switching the primary src to refer to the
     * aux. However, the aux src we need (clksrc_pll_sys) is already selected
     * on reset, so we can also set this in one. */
    CLOCKS_CLK_SYS_CTRL = 1; /* CLK_SYS SRC = clksrc_clk_sys_aux */

    /* Now that the important clocks are running off XOSC, we can disable
     * ROSC to save power. */
    SET_FIELD(ROSC_CTRL, ROSC_CTRL_ENABLE, ROSC_CTRL_ENABLE_DISABLE);

    /* CLK_PERI is set up with aux src 'clk_sys' on reset, so it is already
     * ready to go - we just need to enable it. */
    CLOCKS_CLK_PERI_CTRL = 1 << 11; /* ENABLE */

    /* Finally, we are ready to initialize core 1. */
    init_core1();

    __start();
}


/* NVIC SETUP FUNCTIONS */

/* On Cortex-M0, only the top two bits of each interrupt priority are
implemented, but for portability priorities should be specified with
integers in the range [0..255].  On Cortex-M4, the top three bits are
implemented.*/

/* irq_priority -- set priority for an IRQ to a value [0..255] */
void irq_priority(int irq, unsigned prio)
{
    if (irq < 0)
        SET_BYTE(SCB_SHPR[(irq+12) >> 2], irq & 0x3, prio);
    else
        SET_BYTE(NVIC_IPR[irq >> 2], irq & 0x3, prio);
}
     
/* See hardware.h for macros enable_irq, disable_irq, 
clear_pending, reschedule */


/*  INTERRUPT VECTORS */

/* We use the linker script to define each handler name as an alias
for default_handler if it is not defined elsewhere.  Applications can
subsitute their own definitions for individual handler names like
uart_handler(). */

/* delay_loop -- timed delay */
void delay_loop(unsigned usecs)
{
    unsigned done_l = TIMER_TIMELR + usecs; /* note that unsigned overflow is defined behavior */
    unsigned done_h = TIMER_TIMEHR + 2;
    /* No need to use latched accesses for the checks, since we don't care
     * about the overall timer value, only the components individually. */
    while (TIMER_TIMERAWL < done_l && TIMER_TIMERAWH < done_h) {
        /* The RP2040 does not guarantee an exact clock rate, but we can
         * nonetheless add some delay here to avoid constant timer accesses. */
        nop(); nop(); nop(); nop();
        nop(); nop(); nop(); nop();
    }
}

/* spin -- show Seven Stars of Death */
void spin(void)
{
    intr_disable();

    gpio_dir(GPIO_LED, 1);
    while (1) {
        gpio_out(GPIO_LED, 1);
        gpio_out(GPIO_LED, 0);
    }          
}

void default_handler(void) __attribute((weak, alias("spin")));

/* The linker script makes all these handlers into weak aliases for */
/* default_handler. */

void nmi_handler(void);
void hardfault_handler(void);
void svc_handler(void);
void pendsv_handler(void);
void systick_handler(void);
void timer0_handler(void);
void timer1_handler(void);
void timer2_handler(void);
void timer3_handler(void);
void uart0_handler(void);
void uart1_handler(void);
void i2c0_handler(void);
void i2c1_handler(void);
void rtc_handler(void);

/* This vector table is placed at address 0 in the flash by directives
in the linker script. */

void *__vectors[] __attribute((section(".vectors"))) = {
    __stack,                    /* -16 */
    __reset,
    nmi_handler,
    hardfault_handler,
    0,            		/* -12 */
    0,
    0,
    0,
    0,            		/*  -8 */
    0,
    0,
    svc_handler,
    0,            		/* -4 */
    0,
    pendsv_handler,
    systick_handler,
    
    /* external interrupts */
    timer0_handler,        /*  0 */
    timer1_handler,
    timer2_handler,
    timer3_handler,
    0,                /*  4 */
    0,
    0,
    0,
    0,             /*  8 */
    0,
    0,
    0,
    0,               /* 12 */
    0,
    0,
    0,
    0,                /* 16 */
    0,
    0,
    0,
    uart0_handler,               /* 20 */
    uart1_handler,
    0,
    i2c0_handler,
    i2c1_handler,               /* 24 */
    rtc_handler,
    0,
    0,
    0,            		/* 28 */
    0,
    0,
    0
};
