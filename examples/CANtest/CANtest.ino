// -------------------------------------------------------------
// CANtest for Teensy 3.1
// by teachop
//

#include <FlexCAN.h>

int led = 13;
FlexCAN CANbus(125000);
static CAN_message_t msg,rxmsg;
static uint8_t hex[17] = "0123456789abcdef";


// -------------------------------------------------------------
static void hexDump(uint8_t dumpLen, uint8_t *bytePtr) {
  uint8_t working;
  while( dumpLen-- ) {
    working = *bytePtr++;
    Serial.write( hex[ working>>4 ] );
    Serial.write( hex[ working&15 ] );
  }
  Serial.write('\r');
  Serial.write('\n');
}


// -------------------------------------------------------------
void setup(void)
{
  CANbus.begin();
  pinMode(led, OUTPUT);     
  
  Serial.println(F("Hello Teensy 3.1 CAN Test."));
}


// -------------------------------------------------------------
void loop(void)
{
  if ( CANbus.read(rxmsg) ) {
    digitalWrite(led, 1);
    hexDump( sizeof(rxmsg), (uint8_t *)&rxmsg );
    msg.len = 8;
    msg.id = 0x555;
    for( int loop=0; loop<8; ++loop ) {
      msg.buf[loop] = tolower(rxmsg.buf[loop]);
    }
    CANbus.write(msg);
    //hexDump( sizeof(msg), (uint8_t *)&msg );
    delay(10);
  } else {
    digitalWrite(led, 0);
  }
}

