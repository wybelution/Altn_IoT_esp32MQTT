/*
  demo BBR display
*/

/* from user_setup.h
//available on left side of esp32mini: 26,18,19,23
#define TFT_MISO 19   //not needed (display has no input)
#define TFT_MOSI 26   //was 23,originally 15
#define TFT_SCLK 18   //originally 14
#define TFT_CS   19   // default 15, was 22, Chip select control pin
#define TFT_DC   23   // default 2, was 21, Data Command control pin
//#define TFT_RST  17 // default 4, Reset pin (could connect to RST pin)
#define TFT_RST  -1   // Set TFT_RST to -1 if display RESET is connected to ESP32 board RST
*/


#include "SPI.h"
#include "TFT_eSPI.h"
#include "Free_Fonts.h" // Include the header file attached to this sketch
#include "ADS1X15.h"  //library for A-D converter for voltage measurements

TFT_eSPI tft = TFT_eSPI();

ADS1115 ADS(0x48);  //using I2C bus
const uint16_t adsGainFactor = 125; //1 bit equals 0,125mV
const uint16_t adsBridgePwrIn = 77; //divider has 10k over 67k; source voltage is 77/10
const uint16_t adsBridgePwrOut = 20; //devider has 10k over 10k; output voltage is 20/10

unsigned long total = 0;
unsigned long tn = 0;

void displayWarningMessage (char * message) {
//24 pixels high

  tft.fillRect(1, 1, tft.width()-2, 26, TFT_BLACK);
  tft.setTextColor(TFT_RED, TFT_BLACK);
  tft.setTextDatum(TL_DATUM);
  tft.drawString(message, 1, 1, FONT4);
}//end of displayWarningMessage()

void displayStatusMessage (char * message) {

  tft.fillRect(1, tft.height()-19, tft.width()-2, 18, TFT_BLACK);
  tft.setTextColor(TFT_GREEN, TFT_BLACK);
  tft.setTextDatum(TL_DATUM);
  tft.drawString(message, 1, tft.height()-19, FONT2);
}//end of displayStatusMessage()

void displayBBRstatus(int status) {
//1st of 3 tiles of status info

  tft.setFreeFont(FSSB18);
  tft.setTextDatum(MC_DATUM);
  tft.setTextPadding(64);
    
  if (status == 1) {
    tft.setTextColor(TFT_BLACK, TFT_GREEN);
    tft.drawString("OK", 31,28+32);
  }
  else {
    tft.setTextColor(TFT_RED, TFT_GREEN);
    tft.drawString("X", 31,28+32);
  }
}//end of displayBBRstatus()

void displayWifiStatus(int status) {
//2nd of 3 tiles of status info

  tft.setFreeFont(FSSB18);
  tft.setTextDatum(MC_DATUM);
  tft.setTextPadding(64);
    
  if (status == 1) {
    tft.setTextColor(TFT_BLACK, TFT_GREEN);
    tft.drawString("OK", 31,28+32+64);
  }
  else {
    tft.setTextColor(TFT_RED, TFT_GREEN);
    tft.drawString("X", 31,28+32+64);
  }
}//end of displayWifiStatus()

void displayBrokerstatus(int status) {
//3rd of 3 tiles of status info

  tft.setFreeFont(FSSB18);
  tft.setTextDatum(MC_DATUM);
  tft.setTextPadding(64);
    
  if (status == 1) {
    tft.setTextColor(TFT_BLACK, TFT_GREEN);
    tft.drawString("OK", 31,28+32+(2*64));
  }
  else {
    tft.setTextColor(TFT_RED, TFT_GREEN);
    tft.drawString("X", 31,28+32+(2*64));
  }
}//end of displayBrokerstatus()

void displayMQTTSpeed (int speed) {
  char speedStr[8];
  itoa(speed, speedStr, 10);
  
  tft.setFreeFont(FF57);
  tft.setTextDatum(ML_DATUM);
  tft.setTextPadding(200);
  tft.setTextColor(TFT_BLUE, TFT_BLACK);
  tft.drawString(speedStr, 110, 80);
}

void displayBatteryStatus(uint16_t percentage) {
  //TODO: log MQTT errors when voltage is dropping
  int capacity=90; //max number of pixels
  const uint16_t startX=64+128+20+1+4;//4 pixels next to inner body
  const uint16_t endX=startX+capacity;
  const uint16_t startY=29+64+64+10+1+4;
  const uint16_t heightY=44-2-(2*4);//inner body(44) - 4 pixel margin
  int attempts=0;
  char voltString[24];
  
  if (percentage ==0) { //non-demo mode, real measurements...

    while (!ADS.isReady() && attempts>=2) {
      attempts++;
      delay(10); //10 ms
    }//if not ready within 3 attempts, then funny values will appear
    
    uint32_t board33V = (uint32_t)ADS.readADC(0);  
    uint32_t pwr5Vout = (uint32_t)ADS.readADC(1);  
    uint32_t batt20_30Vout = (uint32_t)ADS.readADC(2);  
    float f_volt=0.0;

    board33V = (board33V * adsGainFactor)/10E3;  //26275 * 125 = 3.284.375
    pwr5Vout = (pwr5Vout * adsGainFactor * adsBridgePwrOut)/10E4; //19550 * 125 * 20) = 48875000
    batt20_30Vout = (batt20_30Vout * adsGainFactor * adsBridgePwrIn)/10E4; //30433 * 125 * 76=289113500
    percentage = map(constrain(batt20_30Vout, 2100, 2500), 2100, 2500, 0, 100); //battery should be between 21 en 25.5 V
    Serial.print("battery = "); Serial.println(batt20_30Vout);
    Serial.print("percent = "); Serial.println(percentage);
    Serial.print("ESP 3.3 = "); Serial.println(board33V);
    Serial.print("PWR 5.0 = "); Serial.println(pwr5Vout);
    Serial.println();
    tft.setTextFont(1);
//    tft.setFreeFont(TT1);
    tft.setTextColor(TFT_YELLOW, TFT_OLIVE);

    tft.setTextDatum(MC_DATUM);
    tft.setTextPadding(36);

    tft.setTextPadding(96);
    snprintf(voltString, 24, "B: %d.%02d     %d%%", batt20_30Vout/100, batt20_30Vout%100, percentage);
    tft.drawString(voltString, startX+capacity/2, startY+heightY+10);

    tft.setTextColor(TFT_YELLOW, TFT_OLIVE);
    tft.setTextPadding(94);
    snprintf(voltString, 24,"i:%d.%01d     o:%d.%01d", board33V/100, board33V%100, pwr5Vout/100, pwr5Vout%100);
    tft.drawString(voltString, startX+capacity/2, startY-8);
  }
  
  capacity=(capacity*percentage)/100; //number of pixels to show
  tft.fillRect(endX-capacity, startY, capacity, heightY, TFT_YELLOW);
  tft.fillRect(startX, startY, endX-startX-capacity, heightY, TFT_BLACK);

  for (uint16_t i=startX; i<endX; i=i+10) {
    tft.drawRect(i, startY, 2, heightY, TFT_BLACK);
    yield();
  }

 
}//end of displayBatteryStatus()

void initBBRGardianDisplay() {
  tft.init();
  tft.fillScreen(TFT_BLACK);
  tft.setRotation(1);
  tft.setTextColor(TFT_NAVY);

  tft.setTextSize(1);
  tft.setTextDatum(TL_DATUM);

  tft.drawRect(0, 0, tft.width(), 28, TFT_RED); //warning message area
  tft.drawRect(0, tft.height()-20, tft.width(), 20, TFT_GREEN); //message area

  tft.fillRoundRect (0,    29,  63,  63,  10, TFT_GREEN); //BBR status
  tft.drawString ( "UART", 5, 29+64-12, GFXFF);
  
  tft.fillRoundRect (0, 29+64,  63,  63,  10, TFT_GREEN); //Wifi status
  tft.drawString ( "WiFi/Int", 5, 29+(2*64)-12, GFXFF);
  
  tft.fillRoundRect (0, 29+64+64, 63, 63, 10, TFT_GREEN); //Broker status
  tft.drawString ( "MQTT", 5, 29+(3*64)-12, GFXFF);

  tft.fillRect ( 64, 29+64+64, 256, 64, TFT_OLIVE); //background of bottom status windows
  tft.drawRect ( 64, 29+64+64, 128, 64, TFT_GREEN); //ESP speed window
  tft.drawRect ( 64+128, 29+64+64, 128, 64, TFT_GREEN); //battery status window

  tft.drawRect (64+128+20, 29+64+64+10, 100, 44, TFT_GREEN); //main body battery
  tft.fillRect (64+128+20+1, 29+64+64+10+1, 100-2, 44-2, TFT_BLACK);
  tft.drawRect (64+128+20+1, 29+64+64+10+1, 100-2, 44-2, TFT_GREEN);//inside main body
  tft.fillRect (64+128+12, 29+64+64+20, 10, 22, TFT_BLACK); //tip of battery
  tft.drawRect (64+128+12, 29+64+64+20, 10, 22, TFT_GREEN); //tip of battery

  displayBBRstatus(1);
  displayWifiStatus(0);
  displayBrokerstatus(1);
  displayBBRstatus(0);
  displayWifiStatus(1);
  displayBrokerstatus(0);
  displayBatteryStatus(100);delay(500);
  
  displayWarningMessage("---warning message---");
  displayStatusMessage("---status message ---");
  displayMQTTSpeed(111); delay(500);
  displayMQTTSpeed(222); delay(500);
  displayMQTTSpeed(333);  delay(500);
  displayMQTTSpeed(666);  delay(500);
  displayMQTTSpeed(999);  delay(500);
  
}//end of initBBRGardianDisplay()

void setup() {
  Serial.begin(115200);
  while (!Serial);
  Serial.flush();
  Serial.println();

  ADS.begin();
  ADS.readADC(0);
  ADS.setGain(1); //PGA gain factor for voltages upto 4.1V

  tn = micros();

  initBBRGardianDisplay();
} //end setup()

void loop(void) {
  displayBatteryStatus(0);
  delay(500);yield();

} //end loop()
