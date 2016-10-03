/*********************************************************************
  This is a modified Adafruit library for the 3-bit Color SHARP Memory Displays.

  Adafruit invests time and resources providing this open source code,
  please support Adafruit and open-source hardware by purchasing
  products from Adafruit!

    Pick one up today in the adafruit shop!
  ------> http://www.adafruit.com/products/1393

  Written by Limor Fried/Ladyada for Adafruit Industries.
  BSD license, check license.txt for more information

 *********************************************************************

  Modified by John Schnurr, Sept 2016. Modified to use the Hardware SPI   Pins.
  Faster frames can
  be
  achieved using the SPI pins.
  Verified to work on a Mega, Due, and Zero.

  The Hardware pins required are:
                     MOSI              SCK                SS
  Arduino Mega:     51 (or ICSP 4)    52 (or ICSP 3)     Any Pin
  Arduino Due:      ICSP 4            ICSP 3             Any Pin
  Arduino Mega:     ICSP 4            ICSP 3             Any Pin

  You may need to adjust the SPI Clock Divisor to get the fastest operation.
  Although the Display is rated for 1Mhz, I was able to use a divisor of 8
  on the Due (12 MHz) and a divisor of 6 on the Zero (8 MHz).

  You can change the SHARPMEM_LCDWIDTH and SHARPMEM_LCDHEIGHT in
  Sharp_Color_LCD.h for the size of display you are using.

*********************************************************************/

#include "Sharp_Color_LCD.h"
#include <SPI.h>

#ifndef _swap_int16_t
#define _swap_int16_t(a, b) { int16_t t = a; a = b; b = t; }
#endif
#ifndef _swap_uint16_t
#define _swap_uint16_t(a, b) { uint16_t t = a; a = b; b = t; }
#endif

/**************************************************************************
    Adafruit Sharp Memory Display Connector
    -----------------------------------------------------------------------
    Pin   Function        Notes
    ===   ==============  ===============================
      1   VIN             3.3-5.0V (into LDO supply)
      2   3V3             3.3V out
      3   GND
      4   SCLK            SPI Hardware Serial Clock Pin
      5   MOSI            SPI Hardware Serial Data Input Pin
      6   CS              Serial Chip Select (can be any pin)
      9   EXTMODE         COM Inversion Select (Low = SW clock/serial)
      7   EXTCOMIN        External COM Inversion Signal
      8   DISP            Display On(High)/Off(Low)

 **************************************************************************/

#define SHARPMEM_BIT_WRITECMD   (0x01)
#define SHARPMEM_BIT_VCOM       (0x02)
#define SHARPMEM_BIT_CLEAR      (0x04)
#define TOGGLE_VCOM             do { _sharpmem_vcom = _sharpmem_vcom ? 0x00 : SHARPMEM_BIT_VCOM; }while (0);

byte sharpmem_buffer[(SHARPMEM_LCDWIDTH * SHARPMEM_LCDHEIGHT /8 ) *3];

/* ************* */
/* CONSTRUCTORS  */
/* ************* */
Sharp_Color_LCD::Sharp_Color_LCD(uint8_t ss) :
  Adafruit_GFX(SHARPMEM_LCDWIDTH, SHARPMEM_LCDHEIGHT) {

  _ss = ss;
  digitalWrite(_ss, HIGH);
  pinMode(_ss, OUTPUT);

  // Set the vcom bit to a defined state
  _sharpmem_vcom = SHARPMEM_BIT_VCOM;
}

void Sharp_Color_LCD::begin() {
  setRotation(2);

#if defined(__SAM3X8E__)
  SPI.begin(_ss);
  SPI.setClockDivider(42); // may have to adjust this clock divisor for best operation
  SPI.setDataMode(SPI_MODE0);
  SPI.setBitOrder(LSBFIRST);
#else
  SPI.begin();
  SPI.setClockDivider(24);  //may have to adjust this clock divisor for best operation
  SPI.setDataMode(SPI_MODE0);
  SPI.setBitOrder(LSBFIRST);
#endif

  clearDisplay();
}

static const byte PROGMEM reverse[256] = {
  0x00, 0x80, 0x40, 0xC0, 0x20, 0xA0, 0x60, 0xE0, 0x10, 0x90, 0x50, 0xD0, 0x30, 0xB0, 0x70, 0xF0,
  0x08, 0x88, 0x48, 0xC8, 0x28, 0xA8, 0x68, 0xE8, 0x18, 0x98, 0x58, 0xD8, 0x38, 0xB8, 0x78, 0xF8,
  0x04, 0x84, 0x44, 0xC4, 0x24, 0xA4, 0x64, 0xE4, 0x14, 0x94, 0x54, 0xD4, 0x34, 0xB4, 0x74, 0xF4,
  0x0C, 0x8C, 0x4C, 0xCC, 0x2C, 0xAC, 0x6C, 0xEC, 0x1C, 0x9C, 0x5C, 0xDC, 0x3C, 0xBC, 0x7C, 0xFC,
  0x02, 0x82, 0x42, 0xC2, 0x22, 0xA2, 0x62, 0xE2, 0x12, 0x92, 0x52, 0xD2, 0x32, 0xB2, 0x72, 0xF2,
  0x0A, 0x8A, 0x4A, 0xCA, 0x2A, 0xAA, 0x6A, 0xEA, 0x1A, 0x9A, 0x5A, 0xDA, 0x3A, 0xBA, 0x7A, 0xFA,
  0x06, 0x86, 0x46, 0xC6, 0x26, 0xA6, 0x66, 0xE6, 0x16, 0x96, 0x56, 0xD6, 0x36, 0xB6, 0x76, 0xF6,
  0x0E, 0x8E, 0x4E, 0xCE, 0x2E, 0xAE, 0x6E, 0xEE, 0x1E, 0x9E, 0x5E, 0xDE, 0x3E, 0xBE, 0x7E, 0xFE,
  0x01, 0x81, 0x41, 0xC1, 0x21, 0xA1, 0x61, 0xE1, 0x11, 0x91, 0x51, 0xD1, 0x31, 0xB1, 0x71, 0xF1,
  0x09, 0x89, 0x49, 0xC9, 0x29, 0xA9, 0x69, 0xE9, 0x19, 0x99, 0x59, 0xD9, 0x39, 0xB9, 0x79, 0xF9,
  0x05, 0x85, 0x45, 0xC5, 0x25, 0xA5, 0x65, 0xE5, 0x15, 0x95, 0x55, 0xD5, 0x35, 0xB5, 0x75, 0xF5,
  0x0D, 0x8D, 0x4D, 0xCD, 0x2D, 0xAD, 0x6D, 0xED, 0x1D, 0x9D, 0x5D, 0xDD, 0x3D, 0xBD, 0x7D, 0xFD,
  0x03, 0x83, 0x43, 0xC3, 0x23, 0xA3, 0x63, 0xE3, 0x13, 0x93, 0x53, 0xD3, 0x33, 0xB3, 0x73, 0xF3,
  0x0B, 0x8B, 0x4B, 0xCB, 0x2B, 0xAB, 0x6B, 0xEB, 0x1B, 0x9B, 0x5B, 0xDB, 0x3B, 0xBB, 0x7B, 0xFB,
  0x07, 0x87, 0x47, 0xC7, 0x27, 0xA7, 0x67, 0xE7, 0x17, 0x97, 0x57, 0xD7, 0x37, 0xB7, 0x77, 0xF7,
  0x0F, 0x8F, 0x4F, 0xCF, 0x2F, 0xAF, 0x6F, 0xEF, 0x1F, 0x9F, 0x5F, 0xDF, 0x3F, 0xBF, 0x7F, 0xFF
};




/**************************************************************************/
/*!
    @brief Draws a single pixel in image buffer
*/
/**************************************************************************/

void Sharp_Color_LCD::drawPixel(int16_t x, int16_t y, uint16_t color)
{
  if ((x < 0) || (x >= _width) || (y < 0) || (y >= _height)) return;

  switch (rotation) {
    case 1:
      _swap_int16_t(x, y);
      x = WIDTH  - 1 - x;
      break;
    case 2:
      x = WIDTH  - 1 - x;
      y = HEIGHT - 1 - y;
      break;
    case 3:
      _swap_int16_t(x, y);
      y = HEIGHT - 1 - y;
      break;
  }

  int startPixel = (x * 3) % 8;
  int index = ((y * SHARPMEM_LCDWIDTH * 3) / 8) + (x * 3 / 8);

  switch (startPixel) {
    case 0:
      sharpmem_buffer[index] = (sharpmem_buffer[index] & 0b00011111) + (color * 32);
      break;
    case 1:
      sharpmem_buffer[index] = (sharpmem_buffer[index] & 0b10001111) + (color * 16);
      break;
    case 2:
      sharpmem_buffer[index] = (sharpmem_buffer[index] & 0b11000111) + (color * 8);
      break;
    case 3:
      sharpmem_buffer[index] = (sharpmem_buffer[index] & 0b11100011) + (color * 4);
      break;
    case 4:
      sharpmem_buffer[index] = (sharpmem_buffer[index] & 0b11110001) + (color * 2);
      break;
    case 5:
      sharpmem_buffer[index] = (sharpmem_buffer[index] & 0b11111000) + (color);
      break;
    case 6:
      sharpmem_buffer[index] = (sharpmem_buffer[index] & 0b11111100) + (color >> 1);
      sharpmem_buffer[index + 1] = (sharpmem_buffer[index + 1] & 0b01111111) + (color << 7);
      break;
    case 7:
      sharpmem_buffer[index] = (sharpmem_buffer[index] & 0b11111110) + (color >> 2);
      sharpmem_buffer[index + 1] = (sharpmem_buffer[index + 1] & 0b00111111) + (color << 6);
      break;
  }

}

/**************************************************************************/
/*!
    @brief Clears the screen
*/
/**************************************************************************/
void Sharp_Color_LCD::clearDisplay()
{
  memset(sharpmem_buffer, 0xff, (SHARPMEM_LCDWIDTH * SHARPMEM_LCDHEIGHT * 3) / 8);
  // Send the clear screen command rather than doing a HW refresh (quicker)
  digitalWrite(_ss, HIGH);
  SPI.transfer(_sharpmem_vcom | SHARPMEM_BIT_CLEAR); //change to LSB
  SPI.transfer(0x00);
  TOGGLE_VCOM;
  digitalWrite(_ss, LOW);
}

/**************************************************************************/
/*!
    @brief Clears the buffer
*/
/**************************************************************************/
void Sharp_Color_LCD::clearBuffer()
{
  memset(sharpmem_buffer, 0xff, (SHARPMEM_LCDWIDTH * SHARPMEM_LCDHEIGHT * 3) /
         8);
}

/**************************************************************************/
/*!
    @brief Renders the contents of the pixel buffer on the LCD
*/
/**************************************************************************/
void Sharp_Color_LCD::refresh(void)
{
  uint16_t i, totalbytes, currentline, oldline;
  totalbytes = (SHARPMEM_LCDWIDTH * SHARPMEM_LCDHEIGHT * 3) / 8;

  // Send the write command
  digitalWrite(_ss, HIGH);
  SPI.transfer(SHARPMEM_BIT_WRITECMD | _sharpmem_vcom);
  TOGGLE_VCOM

  // Send the address for line 1
  oldline = currentline = 1;
  SPI.transfer(currentline);

  SPI.setBitOrder(MSBFIRST);

  // Send image buffer
  for (i = 0; i < totalbytes; i++)
  {
    SPI.transfer(sharpmem_buffer[i]);
    currentline = ((i + 1) / (SHARPMEM_LCDWIDTH * 3 / 8)) + 1;
    if (currentline != oldline)
    {
      // Send end of line and address bytes
      SPI.transfer(0x00);
      if (currentline <= SHARPMEM_LCDHEIGHT)
      {
        SPI.transfer(reverse[currentline]);
      }
      oldline = currentline;
    }
  }
  SPI.setBitOrder(LSBFIRST);
  // Send another trailing 8 bits for the last line
  SPI.transfer(0x00);
  digitalWrite(_ss, LOW);
}



