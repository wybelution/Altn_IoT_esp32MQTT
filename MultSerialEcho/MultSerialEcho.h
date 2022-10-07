//
//DEBUG utilities
//

#define DEBUG 1 //if DEBUG is false, no additional debug messages will be sent to Serial

#if DEBUG
  #define PRINTS(s)   { Serial.print(F(s)); }
  #define PRINTSV(s,v)  { Serial.print(F(s)); Serial.print(v); }
  #define PRINTV(v)   { Serial.print(v); }
  #define PRINTX(s,v) { Serial.print(F(s)); Serial.print(F("0x")); Serial.print(v, HEX); }
#else
  #define PRINTS(s)
  #define PRINTSV(s,v)
  #define PRINTV(v) 
  #define PRINTX(s,v)
#endif

