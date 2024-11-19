// microbian/adc.c
// Copyright (c) 2021 J. M. Spivey

#include "microbian.h"
#include "hardware.h"

static int ADC;

static void adc_task(int dummy) {
    int client, chan;
    short result;
    message m;

#ifdef UBIT_V1
    // Initialise the ADC: 10 bit resolution,
    // compare 1/3 of the input with 1/3 of Vdd
    ADC_CONFIG = FIELD(ADC_CONFIG_RES, ADC_RES_10bit)
        | FIELD(ADC_CONFIG_INPSEL, ADC_INPSEL_AIn_1_3)
        | FIELD(ADC_CONFIG_REFSEL, ADC_REFSEL_Vdd_1_3);
    ADC_INTEN = BIT(ADC_INT_END);
#endif

#ifdef UBIT_V2
    // Initialise the SAADC: 10 bit resolution,
    // compare 1/4 if the input with 1/4 of Vdd with
    // acquisition window of 10 microsec.  (Yes, micro not pico.)
    ADC_CHAN[0].CONFIG = FIELD(ADC_CONFIG_GAIN, ADC_GAIN_1_4)
        | FIELD(ADC_CONFIG_REFSEL, ADC_REFSEL_VDD_1_4)
        | FIELD(ADC_CONFIG_TACQ, ADC_TACQ_10us);
    ADC_RESOLUTION = ADC_RESOLUTION_10bit;
    ADC_INTEN = BIT(ADC_INT_END) | BIT(ADC_INT_CALDONE);
#endif

#ifndef PI_PICO
    connect(ADC_IRQ);
    enable_irq(ADC_IRQ);
#endif
 
#ifdef UBIT_V2
    // Run a calibration cycle to set zero point
    ADC_ENABLE = 1;
    ADC_CALIBRATE = 1;
    receive(INTERRUPT, NULL);
    assert(ADC_CALDONE);
    ADC_CALDONE = 0;
    clear_pending(ADC_IRQ);
    enable_irq(ADC_IRQ);
    ADC_ENABLE = 0;
#endif

    while (1) {
        receive(ANY, &m);
        assert(m.type == REQUEST);
        client = m.sender;
        chan = m.int1;

#ifdef UBIT_V1
        SET_FIELD(ADC_CONFIG, ADC_CONFIG_PSEL, BIT(chan));
        ADC_ENABLE = 1;
        ADC_START = 1;
        receive(INTERRUPT, NULL);
        assert(ADC_END);
        result = ADC_RESULT;
        ADC_END = 0;
        ADC_ENABLE = 0;
#endif

#ifdef UBIT_V2
        ADC_CHAN[0].PSELP = chan+1;
        ADC_ENABLE = 1;
        ADC_RESULT.PTR = &result;
        ADC_RESULT.MAXCNT = 1;
        ADC_START = 1;
        ADC_SAMPLE = 1;
        receive(INTERRUPT, NULL);
        assert(ADC_END);
        assert(ADC_RESULT.AMOUNT == 1);
        ADC_END = 0;
        ADC_ENABLE = 0;

        // Result can still be slightly negative even after calibration
        if (result < 0) result = 0;
#endif

#ifdef PI_PICO
//not sorted out FIFO interrupt FCS mechanism yet so just yield() from busy 
     //chan is pin or virtual pin if temperature sensor
      //ADC_CS should be 0 when we enter here
      SET_FIELD(ADC_CS, ADC_CS_AINSEL, chan);
      SET_BIT(ADC_CS, ADC_CS_EN);

      SET_BIT(ADC_CS, ADC_CS_START_ONCE);
      while (GET_BIT(ADC_CS, ADC_CS_READY) == 0)
      {
         yield();
         if (GET_BIT(ADC_CS, ADC_CS_ERR) == 1)
         {//handle error here
          //for now just break;
             break;
         }
      }
      result = ADC_RESULT;

      ADC_CS = 0;

#endif

#ifndef PI_PICO
        clear_pending(ADC_IRQ);
        enable_irq(ADC_IRQ);
#endif

        send_int(client, REPLY, result);
    }
}

/* chantab -- translate pin numbers to ADC channels */
static const int chantab[] = {
#ifdef UBIT_V1
    PAD0, 4, PAD1, 3, PAD2, 2, PAD3, 5, PAD4, 6, PAD10, 7,
#endif
#ifdef UBIT_V2
    PAD0, 0, PAD1, 1, PAD2, 2, PAD3, 7, PAD4, 4, PAD10, 6,
#endif
#ifdef PI_PICO
//ADCT is a special value
    GPIO_26_ADC0, MUX_ADC0,
    GPIO_27_ADC1, MUX_ADC1,
    GPIO_28_ADC2, MUX_ADC2,
    GPIO_29_ADC3, MUX_ADC3,
    GPIO_VIRT_TS, MUX_TEMP,
#endif
    0
};

int adc_reading(int pin) {
    int i, chan = -1;
    message m;

    for (i = 0; chantab[i] != 0; i += 2) {
        if (chantab[i] == pin) {
            chan = chantab[i+1];
            break;
        }
    }

    if (chan < 0)
        panic("Can't use pin %d for ADC", pin);

#ifdef PI_PICO
      unsigned char remember_ie;
      unsigned char remember_od;

      ADC_CS = 0;

      if (chan == GPIO_VIRT_TS)
      {
         SET_BIT(ADC_CS, ADC_CS_TS_EN);
      }
      else
      {
          volatile unsigned *gpio_reg = &PADS_BANK0_GPIO0 + pin;
          remember_ie = GET_BIT(*gpio_reg, PADS_GPIO_IE);
          remember_od = GET_BIT(*gpio_reg, PADS_GPIO_OD);
          CLR_BIT(*gpio_reg, PADS_GPIO_IE);
          SET_BIT(*gpio_reg, PADS_GPIO_OD);
      }
#endif

    m.type = REQUEST;
    m.int1 = chan;
    sendrec(ADC, &m);

#ifdef PI_PICO
      if (chan == GPIO_VIRT_TS)
      {
         CLR_BIT(ADC_CS, ADC_CS_TS_EN);
      }
      else
      {
          volatile unsigned *gpio_reg = &PADS_BANK0_GPIO0 + pin;
          if (remember_ie == 0)
             CLR_BIT(*gpio_reg, PADS_GPIO_IE);
          else
             SET_BIT(*gpio_reg, PADS_GPIO_IE);

          if (remember_od == 0)
             CLR_BIT(*gpio_reg, PADS_GPIO_OD);
          else
             SET_BIT(*gpio_reg, PADS_GPIO_OD);
      }
#endif

    return m.int1;
}

void adc_init(void) {
    ADC = start("ADC", adc_task, 0, 256);
}
