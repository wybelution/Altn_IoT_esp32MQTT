/*
*
*/

#ifdef ESP8266
 #define LED_PIN LED_BUILTIN
 #define LEDON LOW
 #define LEDOFF HIGH
#else
 #define LED_PIN LED_BUILTIN
 #define LEDON HIGH
 #define LEDOFF LOW
#endif

#define ERRPULSE 9  // (9 times, SOS sequence) for critical situations
#define WRNPULSE 5  // (5 times) for non-blocking error situations
#define OKPULSE 2   // (2 times) for OK (connection)
#define SNGLPULSE 1 // (1 time) for progress

#define MAXBUFFER 128    //a typical msg is 86 characters
#define CHRDELIMITER ':' //start of new values in a msg
