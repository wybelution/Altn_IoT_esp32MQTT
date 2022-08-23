/*
 * Sketch to copy serial messages (from BigBalancingRobot) to MQTT
 * 
 * a typical incoming serial msg looks like this: 
 * 
 *    ConT:10.00,-0.00,0.00,0.007,0.005,-0.01,-0.00,-0.00,0.00,-0.02,-0.00,0.42,-0.07,-0.01
 *    
 * time,posX,posY,roll,pitch,yaw,speedX,speedY,rollSpeed,pitchSpeed,yawSpeed,rollTorque,pitchTorque,yawTorque   
 * 
 * in this sketch the serial buffer is read until a ":" is found. The characters after that
 * are read and stored in msgBuffer until either a \n\t or a "C" is found. That buffer
 * is sent to MQTT broker and the loop starts again
 * 
 * by Wybe Horsman
 * wybe.horsman@alten.nl
*/

#include <climits> 
#include "Serial2MQTT.h"

#ifdef ESP8266
  #include <ESP8266WiFi.h>
#else
  #include <WiFi.h>
#endif

#include <ArduinoMqttClient.h>
#include <MQTTSerialPrinter.h>


char g_msgBuffer[MAXBUFFER]="42.00,-0.00,0.00,0.000,0.000,-0.00,-0.00,-0.00,0.00,-0.00,-0.00,0.00,-0.00,-0.00"; //a typical msg is 86 characters
uint32_t g_startTime=0L;


// wifi and MQTT network setup
const char* ssid        = "FaerynetMobile";
const char* password    = "1qazxsw2";
const char* mqtt_broker = "broker.hivemq.com"; // can be ip or domain name
const char* mqtt_topic  = "AltnIoTtopics/BBR2cloud";
const int   mqtt_port   = 1883;

WiFiClient wifi_client;
MqttClient mqtt_client(wifi_client);
MQTTSerialPrinter mqttLog(mqtt_client, mqtt_topic, false); //setup logging, no Serial echo!

void pulseLed (uint16_t times) {
//function to pulse the onboard LED, this is used for visual indicating the status
//ERRPULSE (9 times, SOS sequence) for critical situations
//WRNPULSE (5 times) for non-blocking error situations
//OKPULSE (2 times) for OK (connection)
//SNGLPULSE (1 time) for progress

  digitalWrite(LED_PIN, LEDOFF); //if some process has turned the LED on, switch it off
  delay(50);
  if (times != ERRPULSE) {
    for (uint16_t t=0; t<times; t++){
      digitalWrite(LED_PIN, LEDON);
      delay(35);
      digitalWrite(LED_PIN, LEDOFF);
      delay(200);
    }
  }
    else {
      for (uint16_t cycle=0; cycle<3; cycle++){
        uint16_t sosDelay=50+((cycle % 2)*250); //use MOD operator to distinguish even and odd numbers
        delay(200);
        for (uint16_t t=0; t<3; t++){
          digitalWrite(LED_PIN, LEDON);
          delay(sosDelay);
          digitalWrite(LED_PIN, LEDOFF);
          delay(100+sosDelay);
        }
      }
  }
}//end of pulseLed()


void setupWifi() {
//function to initialise wifi connection
  WiFi.mode(WIFI_STA);  //set normal STATION mode (connect to accesspoint)
  WiFi.begin(ssid, password); 
  mqttLog.println("ESP::Connecting to WiFi...");
  while (WiFi.status() != WL_CONNECTED) {
    pulseLed(SNGLPULSE);
    delay(100);
  }
  mqttLog.println("ESP::WiFi Connected!");
  pulseLed(OKPULSE);
}


void setupMqtt() {
//connect to MQTT broker using predefined connection settings
  if (!mqtt_client.connected()) {
    mqttLog.println("ESP::Connecting to MQTT...");
    if (!mqtt_client.connect(mqtt_broker, mqtt_port)) {
      mqttLog.print("ESP::MQTT connection failed! Error code = ");
      mqttLog.println(mqtt_client.connectError());
      Serial.println(F("ESP::MQTT connection failed!"));
      while(true) {//loop forever, sketch is useless if there is no broker connection
        pulseLed(ERRPULSE); //send out an S.O.S. ...
        delay(1000);
      }
    } 
    else {
      mqttLog.println("ESP::Connected to MQTT broker!");
      pulseLed(OKPULSE);
    }
  }
}//end of setup_mqtt()


int readDelimiter (){ 
//read Serial port until a delimiter is found
//returns the number of characters read before CHRDELIMITER is found
//returns -1 after maxint 
  char serChar='\0';
  uint16_t counter=0;

  while(!Serial.available() ) delay(0); //wait until a character is ready in the serial buffer
//  mqttLog.println("Serial data available");

  while ((serChar != CHRDELIMITER) && (counter < INT_MAX)) {
    serChar=Serial.read(); //read 1 byte
    counter++;
    while(!Serial.available() ) delay(0); //wait for next serial byte
  }
//  mqttLog.printlnf(">>delimiter found after %d reads"; counter);

  if (counter == INT_MAX) {
    return -1;
  }
  else {
    return counter;
  }
}//end readDelimiter()


char readChar (){ 
//read Serial port until a char is found

  char serChar='Q';

  while(!Serial.available() ) delay(0); //wait until a character is ready in the serial buffer
  serChar=Serial.read(); //read 1 byte

//  mqttLog.printlnf(">>char read from serial: %d"; serChar);

  return serChar;
}//end readChar()


uint8_t checkChar (char serChar) {
// check if the character is a BADCHAR, a ENDCHAR or a GOODCHAR
uint8_t result;

  result = GOODCHAR;
  if ((serChar ==0) || (serChar ==10) || (serChar ==13) || (serChar =='C')) {
    result = ENDCHAR; // null, <CR>, <LF> and start of new serial msg
  }
  else if (strchr("-,.0123456789", serChar) == NULL ) { //is the char in expected set?
    result = BADCHAR;
  }
  return result;
}


void sendBufferToMQTT (){
//create MQTT message, send the message only if <interval> time has passed

  if ( millis()-g_startTime > MQTTINTERVAL ) {
    mqttLog.println(g_msgBuffer); //send to MQTT
    Serial.println(g_msgBuffer);
    g_startTime=millis();
  }
}//end of sendBufferToMQTT()

void setup() {
//initial routine to start after boot
  Serial.setRxBufferSize(1024); //default buffer on ESP8266 is 256 bytes
  Serial.begin(9600,SERIAL_8N1);
  delay (100);
  pinMode(LED_BUILTIN, OUTPUT);       // prepare the LED for turning on and off
  digitalWrite(LED_BUILTIN, LOW);    // turn the LED off by making the voltage HIGH...

  setupWifi();
  setupMqtt(); //when returning from the setup, the MQTT connection is functional!
  mqttLog.println("ESP is up and running, MQTT is ready to fire");
  g_startTime=millis();
  mqttLog.println(g_msgBuffer); //test msg in the right format

}//end of setup()


void loop() {
//main() function
//listen to serial port until a begin of a new msg is found
//then copy the rest of the msg and store in a buffer
//when the end of the msg is found, send buffer to MQTT broker
  static uint8_t charStatus;
  static char chr='Q'; //random character
  static uint8_t idx; //index for buffer array
  static int readAttempts;

  readAttempts=readDelimiter(); //find the start of a new serial bytestream
  if (readAttempts == -1) {
    mqttLog.println(">>readDelimiter() giving up");
    Serial.println(">>readDelimiter() giving up");
    return;
  }

  idx=0;
  do { //build the msgBuffer with serial chars until ENDCHAR or BADCHAR is detected
    chr=readChar();

    charStatus=checkChar(chr); //quality check

    switch (charStatus) {
      case BADCHAR : //reset buffer, start from scratch
          Serial.print("BADCHAR detect: "); Serial.println(chr,DEC);
//          mqttLog.printlnf(">>BADCHAR>>%d",chr);
          break;
      case ENDCHAR :  //send MQTT + reset buffer
//          Serial.print("ENDCHAR after: "); Serial.println(idx);
          g_msgBuffer[idx]='\0';//terminate msgBuffer with EndOfString
          sendBufferToMQTT(); //create MQTT msg, send if interval time has passed
          break;
      case GOODCHAR : //add to buffer, continue
          g_msgBuffer[idx]=chr;
          idx++;
          break;
      default :
          break;
    }//end switch
  } while ((charStatus != BADCHAR) && (charStatus != ENDCHAR) && (idx <(MAXBUFFER-1)) );

  // Keep the connection alive and recieve incoming data
  // (not strictly needed in this sample, but good practice)
  //  mqtt_client.poll();

}//end of loop()
