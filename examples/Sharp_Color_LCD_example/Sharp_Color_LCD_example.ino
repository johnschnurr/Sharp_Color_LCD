/*********************************************************************
  This is an Arduino library for the Sharp Color Memory Displays.

  It is based off the Adafruit Library for Sharp Memory Displays.

  ********************************************************************

  Adafruit invests time and resources providing this open source code,
  please support Adafruit and open-source hardware by purchasing
  products from Adafruit!

    Pick one up today in the adafruit shop!
  ------> http://www.adafruit.com/products/1393

  Written by Limor Fried/Ladyada for Adafruit Industries.
  BSD license, check license.txt for more information

 *********************************************************************

  Modified by John Schnurr, Sept 2016. Modified to use the Hardware SPI Pins.
  Faster frames can be achieved using the SPI pins. Also modified to work
  with the 3-bit color displays by Sharp.
  Verified to work on a Due, and Zero.

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


#include <Adafruit_GFX.h>
#include <Sharp_Color_LCD.h>

#define SS 10

Sharp_Color_LCD display(SS);

#define BLACK 0
#define BLUE 1
#define GREEN 2
#define CYAN 3
#define RED 4
#define MAGENTA 5
#define YELLOW 6
#define WHITE 7

unsigned long timer1;
unsigned long timer2;
unsigned long fps;


char* color[8] = {"Black", "Blue", "Green", "Cyan", "Red", "Magenta", "Yellow", "White"};

int color_index;


void setup() {

  display.begin();
  display.setRotation(0);

  for (int h = 0; h < 8; h++) {
    //fill entire screen with a color
    display.fillRect(0, 0, 128, 128, h % 8);
    display.fillRect(20, 20, 45, 10, BLACK);
    display.setCursor(21, 21);
    display.setTextColor(WHITE);
    display.print(color[h % 8]);
    //refresh must be called to send the image buffer to the display
    display.refresh();
    delay(2500);
  }

  for (int h = 0; h < 4; h++) {
    display.setRotation(h);
    for (int m = 0; m < 8; m++) {
      display.fillRect(0, m * 16, 128, 16, m);
    }
    //refresh must be called to send the image buffer to the display
    display.refresh();
    delay(1500);
  }
  display.setRotation(0);
}

void loop() {
  //start timer for fps calculation
  timer1 = micros();


  //draw the scrambling blocks:
  for (int i = 0; i < 4; i++) {
    for (int j = 0; j < 4; j++) {
      display.fillRect(i * 32, j * 32, 32, 32, (i * 2 + j + color_index) % 8);
    }
  }
  color_index++;


  //print the fps calculation:
  display.setCursor(10, 10);
  display.setTextColor(0);
  display.fillRect(9, 9, 55, 9, WHITE );
  display.print(1000000.0 / fps, 2);
  display.print(" fps");
  //refresh must be called to send the image buffer to the display
  display.refresh();
  timer2 = micros();
  fps = timer2 - timer1;

}
