#include "hardware.h"
#include "microbian.h"
#include "lib.h"
#include "ssd1306.h"

int OLED;
typedef unsigned char byte;
typedef unsigned char uint8_t;
typedef unsigned int uint32_t;
typedef unsigned char bool;

#define false (0)
#define true (1)

//currently only I2C0 interface reset released in startup

static void I2cConfig(int bus);
void i2cRead( uint8_t slvAdd, uint8_t regAdd, uint8_t* data, uint32_t len );
int i2c_xfer_surrogate(int chan, int cmd, int addr, byte* p1, int len1, byte* p2, int len2);
//extern inline void gpio_set_func(unsigned pin, unsigned func);
//extern inline void gpio_pullup(unsigned pin);

//BORROW from lib.c
#define NMAX 16                 // Max digits in a printed number

/* utoa -- convert unsigned to decimal or hex */
static char *utoa(unsigned x, unsigned base, char *nbuf) {
    char *p = &nbuf[NMAX];
    const char *hex = "0123456789abcdef";

    *--p = '\0';
    do {
        *--p = hex[x % base];
        x = x / base;
    } while (x != 0);

    return p;
}

/* itoa -- convert signed integer to decimal */
static char *itoa(int v, char *nbuf) {
    if (v >= 0)
        return utoa(v, 10, nbuf);
    else {
        char *p = utoa(-v, 10, nbuf);
        *--p = '-';
        return p;
    }
}
//end of BORROW


static inline void wibble(void)
{
   SIO_GPIO_OUT_XOR = BIT(25);
}


/* configure i2c0 master */
static void I2cConfig(int bus)
{
    (void)bus;
    // Disable I2C
    I2C0_ENABLE = 0;
    // High & Low counts: From RP2040 Datasheet [Fast mode (400kbps) with ref clk @ 12MHz] rescaled to 125MHz

    volatile unsigned *_ctrl = &I2C0_CON;

    SET_BIT(*_ctrl, I2C_CON_MASTER_MODE_ENABLED);
    SET_FIELD(*_ctrl, I2C_CON_SPEED, I2C_CON_SPEED_FAST);

    CLR_BIT(*_ctrl, I2C_CON_10BITADDR_SLAVE);
    CLR_BIT(*_ctrl, I2C_CON_10BITADDR_MASTER);
    CLR_BIT(*_ctrl, I2C_CON_RESTART_EN);
    SET_BIT(*_ctrl, I2C_CON_SLAVE_DISABLE);
    CLR_BIT(*_ctrl, I2C_CON_STOP_DET_IFADDRESSED);
    SET_BIT(*_ctrl, I2C_CON_TX_EMPTY_CTRL);
    CLR_BIT(*_ctrl, I2C_CON_RX_FIFO_FULL_HLD_CTRL);

    I2C0_FS_SCL_HCNT = (71*125)/12;
    I2C0_FS_SCL_LCNT = (16*125)/12;
    I2C0_FS_SPKLEN   = (1*125)/12;
    I2C0_SDA_HOLD    = 0;

    // Set GPIO20 and 21 to function 3 (I2C0)
    gpio_set_func(21, GPIO_FUNC_I2C);
    gpio_set_func(20, GPIO_FUNC_I2C);

    // PullUp Enabled (I2C)
    gpio_pullup(21);
    gpio_pullup(20);

    // Enable I2C
    I2C0_ENABLE = 1;
}

int i2c_xfer_surrogate(int chan, int cmd, int addr, byte* p1, int len1, byte* p2, int len2)
{
   (void)chan;
//   assert(cmd == WRITE);
//   assert(len1 == 1);

    I2C0_ENABLE = 0;
    I2C0_TAR    = addr;
    I2C0_ENABLE = 1;

    I2C0_DATA_CMD = ((I2C_DATA_CMD_RESTART_DISABLE << I2C_DATA_CMD_RESTART) |
                     (I2C_DATA_CMD_STOP_DISABLE << I2C_DATA_CMD_STOP) |
                     (I2C_DATA_CMD_CMD_WRITE << I2C_DATA_CMD_CMD) |
                     *p1 );

    while ( GET_BIT(I2C0_RAW_INTR_STAT, I2C_RAW_INTR_STAT_TX_EMPTY) != I2C_RAW_INTR_STAT_R_TX_EMPTY_ACTIVE);


    int len = len2;
    uint8_t* data = p2;
    for (uint8_t cnt = 0; cnt < len; cnt++ )
    {

       bool lastbyte     = ( ( len - cnt ) == 1 ? true : false );
       if (lastbyte)
       {
       I2C0_DATA_CMD = ((I2C_DATA_CMD_RESTART_DISABLE << I2C_DATA_CMD_RESTART) |
                     (I2C_DATA_CMD_STOP_ENABLE << I2C_DATA_CMD_STOP) |
                     (I2C_DATA_CMD_CMD_WRITE << I2C_DATA_CMD_CMD) |
                     *data++ );
       }
       else
       {
       I2C0_DATA_CMD = ((I2C_DATA_CMD_RESTART_DISABLE << I2C_DATA_CMD_RESTART) |
                        (I2C_DATA_CMD_STOP_DISABLE << I2C_DATA_CMD_STOP) |
                        (I2C_DATA_CMD_CMD_WRITE << I2C_DATA_CMD_CMD) |
                        *data++ );
       }

       while ( GET_BIT(I2C0_RAW_INTR_STAT, I2C_RAW_INTR_STAT_TX_EMPTY) != I2C_RAW_INTR_STAT_R_TX_EMPTY_ACTIVE);

    }

    return OK;
}


/* read (1 Byte) for now*/
void i2cRead( uint8_t slvAdd, uint8_t regAdd, uint8_t* data, uint32_t len )
{
    (void)len;
    I2C0_ENABLE = 0;
    I2C0_TAR    = slvAdd;
    I2C0_ENABLE = 1;

       I2C0_DATA_CMD = ((I2C_DATA_CMD_RESTART_DISABLE << I2C_DATA_CMD_RESTART) |
                        (I2C_DATA_CMD_STOP_DISABLE << I2C_DATA_CMD_STOP) |
                        (I2C_DATA_CMD_CMD_WRITE << I2C_DATA_CMD_CMD) |
                        regAdd );



       while ( GET_BIT(I2C0_RAW_INTR_STAT, I2C_RAW_INTR_STAT_TX_EMPTY) != I2C_RAW_INTR_STAT_R_TX_EMPTY_ACTIVE);

    while ( GET_BIT(I2C0_RAW_INTR_STAT, I2C_RAW_INTR_STAT_STOP_DET) != I2C_RAW_INTR_STAT_R_STOP_DET_ACTIVE );

       I2C0_DATA_CMD = ((I2C_DATA_CMD_RESTART_DISABLE << I2C_DATA_CMD_RESTART) |
                     (I2C_DATA_CMD_STOP_ENABLE << I2C_DATA_CMD_STOP) |
                     (I2C_DATA_CMD_CMD_READ << I2C_DATA_CMD_CMD) );



    while ( !( I2C0_RXFLR) );

    data[0] = I2C0_DATA_CMD & 0xff;
}


void oled_task(int arg)
{
    message m;
    const unsigned LED_PIN = GPIO_LED;
    gpio_set_func(LED_PIN, GPIO_FUNC_SIO);
    gpio_dir(LED_PIN, 1); //1=output

    // config I2C0 Master+Fast mode on gpio20/gpio21
    I2cConfig(0); //only sets up i2c0 at the moment

    ssd1306_start();
    ssd1306_clear_screenX();

    int mode = 0;
    int ts  = 65535;
    int val = 65535;
    char buffer[NMAX];

    timer_pulse(1000);

    while(1)
    {
        receive(ANY, &m);
        switch (m.type) {
        case PING:
/*
           if (mode)
              ssd1306_inverse_screen();
           else
              ssd1306_normal_screen();
*/
           mode = !mode;
           wibble();
           ts = adc_reading(GPIO_VIRT_TS);
           val = adc_reading(GPIO_26_ADC0);

           ssd1306_set_position(0,0);
           ssd1306_draw_string(itoa(val, buffer));
           ssd1306_draw_string(" ");
           ssd1306_set_position(0,1);
           ssd1306_draw_string(itoa(ts, buffer));
           ssd1306_draw_string(" ");
       break;
        default:
	badmesg(m.type);
        }
    }
}

void init(void)
{
    serial_init();
    timer_init();
    adc_init();
    OLED = start("oled", oled_task, 0, STACK);
}
