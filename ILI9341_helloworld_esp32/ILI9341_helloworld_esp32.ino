#include <Arduino_GFX_Library.h>
 
#define TFT_SCK    18
#define TFT_MOSI   23
#define TFT_MISO   19 //not needed!
#define TFT_CS     22
#define TFT_DC     21
#define TFT_RESET  17

Arduino_ESP32SPI bus = Arduino_ESP32SPI(TFT_DC, TFT_CS, TFT_SCK, TFT_MOSI, TFT_MISO);
Arduino_ILI9341 display = Arduino_ILI9341(&bus, TFT_RESET);
 
void setup(void)
{
  display.begin();
  display.setRotation(1); //landscape, connections at right side
  display.fillScreen(BLACK);

  display.setCursor(20, 40);
  display.setTextSize(3);
  display.setTextColor(BLUE);
  display.print("345");
}
 
void loop() {}
