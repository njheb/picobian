// echo.c
// Copyright (c) 2018 J. M. Spivey
#include "hardware.h"
#include "microbian.h"
#include "lib.h"
#include <string.h>

void read_line(char *buf) {
    char *p = buf;

    for (;;) {
        char ch = serial_getc();
        if (ch == '\n') break;
        *p++ = ch;
    }

    *p = '\0';
}

char line[512];

void echo_task(int n) {

    printf("Hello" __DATE__ " " __TIME__ "\n");

    while (1) {
        yield();
        read_line(line);
        printf("--> %s (%d)\n", line, strlen(line));
    }
}
#if 0
void crude_delay(int slots)
{
   while (slots-- > 0){
      delay_loop(10000);  //10ms
      yield();
   }
}
#endif

void blink_task(int n)
{
   message m;
   const unsigned LED_PIN = 25;
   unsigned char state = 0x01;
   gpio_set_func(LED_PIN, GPIO_FUNC_SIO);
   gpio_dir(LED_PIN, 1); //1=output
   //gpio_drive(LED_PIN, GPIO_DRIVE_8MA); //set gpio drive strength? need mode

  timer_pulse(250);

   while (1)
   {
      gpio_out(LED_PIN, state);

      receive(ANY, &m);
      switch(m.type)
      {
         case PING:
          state ^= 0x01;
         break;
         default:
            badmesg(m.type);
      }

   }
}

void init(void) {
    serial_init();
    timer_init();

    start("Echo", echo_task, 0, STACK);
    start("Blink", blink_task, 0, STACK);
}
