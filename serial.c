// serial.c
// Copyright (c) 2018 J. M. Spivey

#include "microbian.h"
#include "hardware.h"
#include <stdarg.h>

#define DEBUG_PIN_IN_SERIAL 9

#ifdef DEBUG_PIN_IN_SERIAL
#define debug_in_serial(b) gpio_out(DEBUG_PIN_IN_SERIAL, b)
#else
#define debug_in_serial(b)
#endif

#ifdef UBIT
#define TX USB_TX
#define RX USB_RX
#endif

static int SERIAL_TASK;

/* Message types for serial task */
#define PUTC 16
#define GETC 17
#define PUTBUF 18

/* There are two buffers, one for characters waiting to be output, and
another for input characters waiting to be read by other processes.
The input buffer has |n_edit| characters in the current line, still
subject to editing, and |n_avail| characters in previous lines that
are available to other processes. */

/* NBUF -- size of input and output buffers.  Should be a power of 2. */
#define NBUF 256

/* wrap -- reduce index to range [0..NBUF) */
#define wrap(x) ((x) & (NBUF-1))

/* Input buffer */
static char rxbuf[NBUF];        /* Circular buffer for input */
static int rx_inp = 0;          /* In pointer */
static int rx_outp = 0;         /* Out pointer */
static int n_avail = 0;         /* Number of chars avail for input */
static int n_edit = 0;          /* Number of chars in current line */

/* Output buffer */
static char txbuf[NBUF];        /* Circular buffer for output */
static int tx_inp = 0;          /* In pointer */
static int tx_outp = 0;         /* Out pointer */
static int n_tx = 0;            /* Character count */

static int reader = -1;         /* Process waiting to read */

#if defined(UBIT) || defined(KL25Z)
static int txidle = 1;          /* True if transmitter is idle */
#endif

/* echo -- echo input character */
static void echo(char ch) {
    if (n_tx == NBUF) return;
    txbuf[tx_inp] = ch;
    tx_inp = wrap(tx_inp+1);
    n_tx++;
}

#define CTRL(x) ((x) & 0x1f)

/* keypress -- deal with keyboard character by editing buffer */
static void keypress(char ch) {
    switch (ch) {
    case '\b':
    case 0177:
        /* Delete last character */
        if (n_edit > 0) {
            n_edit--;
            rx_inp = wrap(rx_inp-1);
            /* This doesn't work well with TAB and other control chars */
            echo('\b'); echo(' '); echo('\b');
        }
        break;

    case '\r':
    case '\n':
        /* Make line available to clients */
        if (n_avail + n_edit == NBUF) break;
        rxbuf[rx_inp] = '\n';
        rx_inp = wrap(rx_inp+1);
        n_edit++;
        n_avail += n_edit; n_edit = 0;
        echo('\r'); echo('\n');
        break;
        
    case CTRL('B'):
        /* Print process table dump */
        dump();
        break;

    default:
        /* Ignore other control characters */
        if (ch < 040 || ch >= 0177) break;

        /* Add character to line */
        if (n_avail + n_edit == NBUF) break;
        rxbuf[rx_inp] = ch;
        rx_inp = wrap(rx_inp+1);
        n_edit++;
        echo(ch);
    }
}

/* The clear_pending() call below is needed because the UART interrupt
handler disables the IRQ for the UART in the NVIC, but doesn't disable
the UART itself from sending interrupts.  The pending bit is cleared
on return from the interrupt handler, but that doesn't stop the UART
from setting it again. */

#if defined(UBIT)
/* serial_interrupt -- handle serial interrupt */
static void serial_interrupt(void) {
    if (UART_RXDRDY) {
        char ch = UART_RXD;
        keypress(ch);
        UART_RXDRDY = 0;
    }

    if (UART_TXDRDY) {
        txidle = 1;
        UART_TXDRDY = 0;
    }

    clear_pending(UART_IRQ);
    enable_irq(UART_IRQ);
}
#elif defined(KL25Z)
/* serial_interrupt -- handle serial interrupt */
static void serial_interrupt(void) {
    if (UART0_S1 & BIT(UART_S1_RDRF)) {
        char ch = UART0_D;
        keypress(ch);
    }

    if (UART0_S1 & BIT(UART_S1_TDRE)) {
        txidle = 1;
        CLR_BIT(UART0_C2, UART_C2_TIE);
    }

    clear_pending(UART0_IRQ);
    enable_irq(UART0_IRQ);
}
#elif defined(PI_PICO)
static void serial_interrupt(void) {
    /* Due to the FIFOs, we could have multiple bytes (UARTRXINTR may not even
     * trigger until we reach a certain fill point). Make sure to flush the
     * whole buffer. */
    while (!GET_BIT(UART0_FR, UART_FR_RXFE)) {
        keypress((char)(unsigned char)UART0_DR);
    }

    clear_pending(UART0_IRQ);
    enable_irq(UART0_IRQ);
}
#endif

/* reply -- send reply or start transmitter if possible */
static void reply(void) {
    // Can we satisfy a reader?
    if (reader >= 0 && n_avail > 0) {
        send_int(reader, REPLY, rxbuf[rx_outp]);
        reader = -1;
        rx_outp = wrap(rx_outp+1);
        n_avail--;
    }

    // Can we start transmitting a character?

#if defined(UBIT)
    if (txidle && n_tx > 0) {
        UART_TXD = txbuf[tx_outp];
        tx_outp = wrap(tx_outp+1);
        n_tx--;
        txidle = 0;
    }
#elif defined(KL25Z)
    if (txidle && n_tx > 0) {
        UART0_D = txbuf[tx_outp];
        SET_BIT(UART0_C2, UART_C2_TIE);
        tx_outp = wrap(tx_outp+1);
        n_tx--;
        txidle = 0;
    }
#elif defined(PI_PICO)
    /* We can do this in a loop due to the UART FIFO */
    while (!GET_BIT(UART0_FR, UART_FR_TXFF) && n_tx > 0) {
        UART0_DR = (unsigned char)txbuf[tx_outp];
        tx_outp = wrap(tx_outp+1);
        n_tx--;
    }
    if (n_tx > 0) {
        /* Trigger an interrupt once there's space in the transmit FIFO. */
        SET_BIT(UART0_IMSC, UART_IMSC_TXIM);
    } else {
        /* We've transmitted all data - stop telling us about FIFO space, or
         * we'll be getting this interrupt constantly! */
        CLR_BIT(UART0_IMSC, UART_IMSC_TXIM);
    }
#endif
}

/* queue_char -- add character to output buffer */
static void queue_char(char ch) {
    while (n_tx == NBUF) {
        // The buffer is full -- wait for a space to appear
        debug_in_serial(0);
        receive(INTERRUPT, NULL);
        debug_in_serial(1);
        serial_interrupt();
        reply();
    }

    txbuf[tx_inp] = ch;
    tx_inp = wrap(tx_inp+1);
    n_tx++;
}

/* serial_task -- driver process for UART */
static void serial_task(int arg) {
    message m;
    int client, n;
    char ch;
    char *buf;

#ifdef DEBUG_PIN_IN_SERIAL
    gpio_set_func(DEBUG_PIN_IN_SERIAL, GPIO_FUNC_SIO);
    gpio_dir(DEBUG_PIN_IN_SERIAL, 1);
    gpio_out(DEBUG_PIN_IN_SERIAL, 1);
#endif

#if defined(UBIT)
    UART_ENABLE = UART_ENABLE_Disabled;
    UART_BAUDRATE = UART_BAUDRATE_9600; // 9600 baud
    UART_CONFIG = FIELD(UART_CONFIG_PARITY, UART_PARITY_None);
                                        // format 8N1
    UART_PSELTXD = TX;                  // choose pins
    UART_PSELRXD = RX;
    UART_ENABLE = UART_ENABLE_Enabled;
    UART_STARTTX = 1;
    UART_STARTRX = 1;
    UART_RXDRDY = 0;

    UART_INTENSET = BIT(UART_INT_RXDRDY) | BIT(UART_INT_TXDRDY);
    connect(UART_IRQ);
    enable_irq(UART_IRQ);

    txidle = 1;
#elif defined(KL25Z)
    // enable PLL clock
    SET_FIELD(SIM_SOPT2, SIM_SOPT2_UART0SRC, SIM_SOPT2_SRC_PLL);
    SET_BIT(SIM_SCGC4, SIM_SCGC4_UART0);
    
    // Disable UART before changing registers
    UART0_C2 &= ~(BIT(UART_C2_RE) | BIT(UART_C2_TE));
    
    // set baud rate
    unsigned BR = UART_BAUD_9600;
    SET_FIELD(UART0_BDH, UART_BDH_SBR, BR >> 8);
    UART0_BDL = BR & 0xff;

    // 8N1 format
    UART0_C1 = 0;
    CLR_BIT(UART0_BDH, UART_BDH_SBNS);

    // set mux for rx/tx pins and enable PullUp mode
    pin_function(USB_TX, 2);
    pin_mode(USB_TX, PORT_MODE_PullUp);

    pin_function(USB_RX, 2);
    pin_mode(USB_RX, PORT_MODE_PullUp);

    // Enable UART
    UART0_C2 |= BIT(UART_C2_RE) | BIT(UART_C2_TE);

    SET_BIT(UART0_C2, UART_C2_RIE);
    enable_irq(UART0_IRQ);
    connect(UART0_IRQ);
    enable_irq(UART0_IRQ);

    txidle = 1;
#elif defined(PI_PICO)
    /* Helper functions from microbian.c */
    extern void uart_set_baud(unsigned baud);
    extern void uart_set_format(unsigned char data_bits, unsigned char stop_bits, unsigned char parity);

    gpio_set_func(USB_TX, GPIO_FUNC_UART);
    gpio_set_func(USB_RX, GPIO_FUNC_UART);
    reset_subsystem(RESET_UART0);
    uart_set_baud(9600);
    uart_set_format(8, 1, 0); /* 8N1 */
    /* Enable FIFOs */
    SET_BIT(UART0_LCR_H, UART_LCR_H_FEN);
    /* Enable UART */
    UART0_CR = BIT(UART_CR_UARTEN) | BIT(UART_CR_TXE) | BIT(UART_CR_RXE);

    connect(UART0_IRQ);
    enable_irq(UART0_IRQ);

    /* UARTRXINTR: RX buffer is filled past the trigger level
     * UARTRTINTR: RX buffer is non-empty, and no recent RX
     * UARTTXINTR: TX buffer is less filled than the trigger level */
    UART0_IMSC = BIT(UART_IMSC_RXIM) | BIT(UART_IMSC_RTIM);
    /* We haven't immediately set UARTTXINTR. We'll only set that when we have
     * pending data: otherwise, it'll be asserted constantly!
     * The default trigger levels are 1/2 full for both the TX and RX FIFOs.
     * This is a good level, and will be set now due to the subsystem reset. */
#endif

    while (1) {
        debug_in_serial(0);
        receive(ANY, &m);
        debug_in_serial(1);

        client = m.sender;

        switch (m.type) {
        case INTERRUPT:
            serial_interrupt();
            break;

        case GETC:
            if (reader >= 0)
                panic("Two clients cannot wait for input at once");
            reader = client;
            break;
            
        case PUTC:
            ch = m.int1;
            if (ch == '\n') queue_char('\r');
            queue_char(ch);
            break;

        case PUTBUF:
            buf = m.ptr1;
            n = m.int2;
            for (int i = 0; i < n; i++) {
                char ch = buf[i];
                if (ch == '\n') queue_char('\r');
                queue_char(ch);
            }
            debug_in_serial(0);
            send_msg(client, REPLY);
            debug_in_serial(1);
            break;

        default:
            badmesg(m.type);
        }
          
        reply();
    }
}

/* serial_init -- start the serial driver task */
void serial_init(void) {
    SERIAL_TASK = start("Serial", serial_task, 0, 256);
}

/* serial_putc -- queue a character for output */
void serial_putc(char ch) {
    send_int(SERIAL_TASK, PUTC, ch);
}

/* serial_getc -- request an input character */
char serial_getc(void) {
    message m;
    m.type = GETC;
    sendrec(SERIAL_TASK, &m);
    return m.int1;
}

/* print_buf -- output routine for use by printf */
void print_buf(char *buf, int n) {
    /* Using sendrec() here avoids a potential priority inversion:
       with separate send() and receive() calls, a lower-priority
       client process can block a reply from the device driver. */

    message m;
    m.type = PUTBUF;
    m.ptr1 = buf;
    m.int2 = n;
    sendrec(SERIAL_TASK, &m);
}
