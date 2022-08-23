/*
 * Sketch to copy serial messages (from BigBalancingRobot) to MQTT
 * 
 * a typical msg looks like this: 
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

#include "Serial2MQTT.h"

#ifdef ESP8266
  #include <ESP8266WiFi.h>
#else
  #include <WiFi.h>
#endif

#include <ArduinoMqttClient.h>
#include <MQTTSerialPrinter.h>


char msgBuffer[MAXBUFFER]=""; //a typical msg is 86 characters
const char c_delimiter=CHRDELIMITER; //current delimiter of a msg
uint32_t starttime=0;

// Edit these fields to match your own network setup
const char* ssid        = "Faerynet-IoT-2G";
const char* password    = "FaerynetWPAkey";
const char* mqtt_broker = "broker.hivemq.com"; // can be ip or domain name
const char* mqtt_topic  = "AltnIoTtopics/BBR2cloud";
const int   mqtt_port   = 1883;

WiFiClient wifi_client;
MqttClient mqtt_client(wifi_client);
MQTTSerialPrinter Log(mqtt_client, mqtt_topic, false); //setup logging, no Serial echo!

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
  Log.println("ESP::Connecting to WiFi...");
  while (WiFi.status() != WL_CONNECTED) {
    pulseLed(SNGLPULSE);
    delay(100);
  }
  Log.println("ESP::WiFi Connected!");
  pulseLed(OKPULSE);
}


void setupMqtt() {
//connect to MQTT broker using predefined connection settings
  if (!mqtt_client.connected()) {
    Log.println("ESP::Connecting to MQTT...");
    if (!mqtt_client.connect(mqtt_broker, mqtt_port)) {
      Log.print("ESP::MQTT connection failed! Error code = ");
      Log.println(mqtt_client.connectError());
      Serial.println(F("ESP::MQTT connection failed!"));
      while(true) {//loop forever, sketch is useless if there is no broker connection
        pulseLed(ERRPULSE); //send out an S.O.S. ...
        delay(1000);
      }
    } 
    else {
      Log.println("ESP::Connected to MQTT broker!");
      pulseLed(OKPULSE);
    }
  }
}//end of setup_mqtt()


void setup() {
//initial routine to start after boot
  Serial.begin(9600,SERIAL_8N1);
  delay (100);
  pinMode(LED_BUILTIN, OUTPUT);       // prepare the LED for turning on and off
  digitalWrite(LED_BUILTIN, LOW);    // turn the LED off by making the voltage HIGH...

  setupWifi();
  setupMqtt(); //when returning from the setup, the MQTT connection is functional!
  Log.println("ESP is up and running, MQTT is ready to fire");
  starttime=millis();
}//end of setup()


void loop() {
//main() function
//listen to serial port until a begin of a new msg is found
//then copy the rest of the msg and store in a buffer
//when the end of the msg is found, send buffer to MQTT broker
  static char chr='Q'; //random character
  static uint8_t idx; //index for buffer array

  while(!Serial.available() ) delay(0); //wait until a character is ready in the serial buffer
//  Log.println("Serial data available");

  do {
    chr=Serial.read(); //read 1 byte
  }while (chr!=c_delimiter);

  //delimiter is found, start new msg
  idx=0;
  do {
    while(!Serial.available() ) delay(0); //crucial code! wait until next character is ready in the serial buffer
    chr=Serial.read();
    msgBuffer[idx]=chr;
    idx++;
  }while ((chr!='\0') && (chr!='\n') && (chr!='\t') && (chr!='C') && (idx<(MAXBUFFER-1))); //stop if EOL or start of new msg

  for (uint16_t i=idx; i<MAXBUFFER; i++){ //clear the remaining of the buffer
    msgBuffer[idx]='\0';
  }

  if (millis()-starttime > MQTTINTERVAL) {
    Log.print(idx);Log.print("::");
    Log.println(msgBuffer); //send to MQTT
    starttime=millis();
  }

  // Keep the connection alive and recieve incoming data
  // (not strictly needed in this sample, but good practice)
//  mqtt_client.poll();

}//end of loop()
