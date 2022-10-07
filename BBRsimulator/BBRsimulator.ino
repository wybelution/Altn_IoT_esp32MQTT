/*
 * sketch to simulate the serial output of the BBR robot (by Alten)
 * this sketch uses a set of 500 actual messages taken from BBR and
 * sends those messages with a maintained speed (set by [msgspeed] to
 * serial output
 * 
 * v1 created by W. Horsman, august '22
 * wybe.horsman@alten.nl
 */
#include "BBRsimulator.h"

//the define DEBUG is used (and set in BBRsimulator.h) to echo debug info to Serial monitor
//for production the DEBUG level should be set to 0

void setup() {
    Serial.begin(115200,SERIAL_8N1);
    delay (100);
    pinMode(LED_BUILTIN, OUTPUT);       // prepare the LED for turning on and off
    digitalWrite(LED_BUILTIN, HIGH);    // turn the LED off by making the voltage HIGH...

    PRINTS("BBRsimulator has started\n");
}//end setup()


void loop() {
//send all messages (c_maxsamples) from the [BBRsample] message block, then repeat
//to emulate the actual BBR the messages should be send with speed [msgspeed] Hz
//

  static uint16_t timestamp1, timestamp2; //t1 and t2 mark timestamp before and after block send
  static uint16_t transmittime; //actual time needed to transmit the c_maxsamples messages (without delays)
  static uint16_t waitms=10; //delay between messages
  static float f_waitms; //float needed to exactly measure the delay
  
  timestamp1=millis(); //start stopwatch
  for (uint16_t i=0; i<c_maxsamples; i++) { //cycle through all BBR sample 
    Serial.write( BBRsample[i] ); //send string to serial
    Serial.write( char(13) ); // aka \r or CR 
    Serial.write( char(10) ); //aka \t or LF
    delay(waitms);
  }
  timestamp2=millis(); //stop the time...
  transmittime = (timestamp2 - timestamp1) - ((c_maxsamples-1) * waitms); // calculate the time to clean-send [c_maxsamples] msgs (no delays)
  if (transmittime > period) { //due to serial baudrate it may happen that the required speed cannot be accomplished
    waitms = 0;
  }
  else {
    //calculate the new delay between messages, based on the actual clean time to send all messages
    //and the defined speed that the messages need to be transmitted per second. 
    f_waitms = (float)((period - transmittime ) / c_maxsamples);
    waitms=round(f_waitms);
  }
  
  digitalWrite(LED_BUILTIN, LOW);    // turn the LED on by making the voltage LOW...
  PRINTS(">>maxsamples (500) sent, ");
  PRINTS("prv transmit="); PRINTV(timestamp2 - timestamp1);
  PRINTS(", new waitms="); PRINTV(waitms); PRINTS("ms\n");
  delay(10);
  digitalWrite(LED_BUILTIN, HIGH);    // turn the LED off by making the voltage HIGH...
  
}//end loop()
