/*
 * sketch to echo incoming serial messages to serial monitor
 *  
 * v1 created by W. Horsman, august '22
 * wybe.horsman@alten.nl
*/

// board: WEMOS D1 mini ESP32

#include "SerialEcho.h"

//the define DEBUG is used (and set in BBRsimulator.h) to echo debug info to Serial monitor
//for production the DEBUG level should be set to 0

void setup() {
    Serial.begin(9600,SERIAL_8N1);
    delay (100);
    pinMode(LED_BUILTIN, OUTPUT);       // prepare the LED for turning on and off
    digitalWrite(LED_BUILTIN, HIGH);    // turn the LED off by making the voltage HIGH...

    PRINTS("SerialEcho has started\n");
}//end setup()


void loop() {
//read string from Serial port and echo to Serial monitor, then repeat
//
  static char chr;
  static String str;
  while(!Serial.available() );
    digitalWrite(LED_BUILTIN, LOW);  //LED on
    str = Serial.readString();

//    chr=Serial.read();
//    PRINTS("ESP32>");
//    PRINTV(chr);
//    PRINTS("\n");


    if(str.indexOf("ConT:") > -1){
      PRINTS("[ConT:] found!\n");} 
    else{
      PRINTS("[sos] NOT found\n");
    }

    PRINTS("ESP32>");
    PRINTV(str);


    digitalWrite(LED_BUILTIN, HIGH);  //LED on
  
}//end loop()
