/*
 * sketch to echo incoming serial messages to serial monitor
 *  
 * v1 created by W. Horsman, august '22
 * wybe.horsman@alten.nl
*/

// board: WEMOS D1 mini ESP32 !!

#include "MultSerialEcho.h"

//the define DEBUG is used (and set in BBRsimulator.h) to echo debug info to Serial monitor
//for production the DEBUG level should be set to 0

void setup() {
    Serial2.setRxBufferSize(1024); //default buffer on ESP8266 is 128+256 bytes
//    Serial2.begin(115200,SERIAL_8N1, SERIAL_RX_ONLY, RXD2, TXD2);
    Serial2.begin(115200, SERIAL_8N1, 16, 17); //for ESP32: use 16,17 for Rx/Tx for Serial2
	Serial2.flush();
	
    Serial.begin(115200, SERIAL_8N1);
	Serial.flush();
    delay (100);
    pinMode(LED_BUILTIN, OUTPUT);       // prepare the LED for turning on and off
    digitalWrite(LED_BUILTIN, LOW);    // turn the LED off by making the voltage LOW...

    PRINTS("MultSerialEcho has started\n");
}//end setup()


void loop() {
//read string from Serial2 port and echo to Serial monitor, then repeat
//
  static char chr;
  static uint64_t microtime=0, waittime1000=0, printtime1000=0;
  static uint32_t count=0;
  
//  static String str;

  microtime = esp_timer_get_time();
  while(!Serial2.available() ) delay(0);
  waittime1000 = waittime1000 + (esp_timer_get_time()- microtime);
  
//  digitalWrite(LED_BUILTIN, LOW);  //LED on
  
//  str = Serial.readString();

  microtime = esp_timer_get_time();
  chr=Serial2.read();
//	if ((chr == '\0') || (chr == '\n')) PRINTS("\nESP32>")
//  else PRINTV(chr);
  printtime1000 = printtime1000 + (esp_timer_get_time() - microtime);
  count++;
  
  if (count == 1000) {
    PRINTSV("\nwaittime : ", waittime1000);
    PRINTSV(" (", (100*waittime1000)/(waittime1000+printtime1000));
    PRINTS ("% ,");
    PRINTSV("printtime : ", printtime1000);
    PRINTSV(" (", (100*printtime1000)/(waittime1000+printtime1000));
    PRINTS ("%)");
    waittime1000=0;
    printtime1000=0;
    count=0;
  }

//    digitalWrite(LED_BUILTIN, HIGH);  //LED on
  
}//end loop()
