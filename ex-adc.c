// ex-adc.c
// based on echo.c Copyright (c) 2018 J. M. Spivey

//adc driver is not using interrupts yet to drain results from FIFO yet
//so if there is a latch up everything will stall

#include "hardware.h"
#include "microbian.h"
#include "lib.h"
//#include <string.h>

void measure_task(int n)
{
    (void)n;
    message m;
   const unsigned LED_PIN = GPIO_LED;
   unsigned char state = 0x01;
   gpio_set_func(LED_PIN, GPIO_FUNC_SIO);
   gpio_dir(LED_PIN, 1); //1=output



    printf("Measure Task " __DATE__ " " __TIME__ "\n");

    timer_pulse(1000);

    while (1) {
       gpio_out(LED_PIN, state);

       receive(ANY, &m);
       switch (m.type) {
        case PING:
            state^=0x01;
            int val = adc_reading(GPIO_26_ADC0);
            printf("10K Pot reading=%d\n", val);
            break;
        default:
            badmesg(m.type);
        }

    }
}

void init(void) {
    timer_init();
    serial_init();
    adc_init();

    start("MeasureTask", measure_task, 0, STACK);
}
