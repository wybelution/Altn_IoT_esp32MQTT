/*
  demo ASCII table display
*/

/* from user_setup.h
#define TFT_MISO 19 //not needed
#define TFT_MOSI 23
#define TFT_SCLK 18
#define TFT_CS   22  // default 15, Chip select control pin
#define TFT_DC   21  // default 2, Data Command control pin
//#define TFT_RST  17  // default 4, Reset pin (could connect to RST pin)
#define TFT_RST  -1  // Set TFT_RST to -1 if display RESET is connected to ESP32 board RST
*/

#include "SPI.h"
#include "TFT_eSPI.h"
#include "Free_Fonts.h" // Include the header file attached to this sketch

TFT_eSPI tft = TFT_eSPI();


void setup() {
  Serial.begin(115200);
  while (!Serial);
  Serial.flush();
  Serial.println();
  
  tft.init();
  tft.fillScreen(TFT_BLACK);
  tft.setRotation(2);
  tft.setTextSize(1);
  tft.setTextColor(TFT_WHITE);
  
  tft.setFreeFont(FF61);
  tft.setCursor(1,1);
  tft.setTextDatum(TL_DATUM);


  for (int i=0; i<=255; i++) {
    tft.write(i);
  }
}

void loop(void) {
}
