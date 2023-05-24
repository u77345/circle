#ifndef __ili9341_h__
#define __ili9341_h__

#include <circle/gpiopin.h>
#include <circle/spimaster.h>
#include <circle/screen.h>


/*
Adafruit PiTFT HAT https://www.adafruit.com/product/2455
STMPE610 12 bit touch controller
240x320 LCD 2.4"
ILI9341 display controller
HAT eeprom in I2C (27, 28)

Touch controller specifics
RT_INT          GPIO24      18
RT_CS_3V        #SPI_CE1    26

Display controller specifics
Backlight       GPIO18      12
TFT_DC_3V (WR)  GPIO25      22
TFT_CS_3V       #SPI_CE0    24

SPI shared across touch and display
SCLK_3V         SPI_SCLK    23
MISO            SPI_MISO    21
MOSI_3V         SPI_MOSI    19
*/