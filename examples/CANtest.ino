// -------------------------------------------------------------
// CANtest for Teensy 3.1
// by teachop
//

#include <FlexCAN.h>

int led = 13;
FlexCAN CANbus(125000);
static CAN_message msg;
static uint8_t txScale;


// -------------------------------------------------------------
void setup(void)
{
  CANbus.begin();
  
  Serial.println(F("Hello Teensy 3.1 CAN Test."));

  pinMode(led, OUTPUT);     
}


// -------------------------------------------------------------
void loop(void)
{
  txScale++;
  if ( 10 < txScale ) {
    for( int loop=0; loop<8; ++loop ) {
      msg.buf[loop] = 'A'+loop;
    }
    msg.len = 8;
    msg.id = 0x555;
    CANbus.send(&msg);
  }
  digitalWrite(led, CANbus.recv(&msg));
  delay(25);
  digitalWrite(led, 0);
  delay(25);
}

