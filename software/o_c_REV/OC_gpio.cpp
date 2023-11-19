#include <Arduino.h>
#include <algorithm>
#include "OC_digital_inputs.h"
#include "OC_gpio.h"
#include "OC_options.h"

#if defined(__IMXRT1062__)

// default settings for traditional O_C hardware wired for Teensy 3.2
uint8_t CV1=19, CV2=18, CV3=20, CV4=17;
uint8_t TR1=0, TR2=1, TR3=2, TR4=3;
uint8_t OLED_DC=6, OLED_RST=7, OLED_CS=8;
uint8_t DAC_CS=10, DAC_RST=9;
uint8_t encL1=22, encL2=21, butL=23, encR1=16, encR2=15, butR=14;
uint8_t but_top=4, but_bot=5, but_mid=255, but_top2=255, but_bot2=255;
uint8_t OC_GPIO_DEBUG_PIN1=24, OC_GPIO_DEBUG_PIN2=25;

FLASHMEM
void OC::Pinout_Detect() {
#if defined(ARDUINO_TEENSY41)
  // TODO, read A17 voltage and configure pins
/*
  CV1 = 19;
  CV2 = 18;
  CV3 = 20;
  CV4 = 17;
  TR1 = 0;
  TR2 = 1;
  TR3 = 2;
  TR4 = 3;
  but_top = 5;
  but_bot = 4;
  OLED_DC = 6;
  OLED_RST = 7;
  OLED_CS = 8;
  DAC_CS = 10;
  DAC_RST = 9;
  encR1 = 16;
  encR2 = 15;
  butR  = 14;
  encL1 = 22;
  encL2 = 21;
  butL  = 23;
  OC_GPIO_DEBUG_PIN1 = 24;
  OC_GPIO_DEBUG_PIN2 = 25;
*/




#endif // ARDUINO_TEENSY41
}

#endif // __IMXRT1062__
