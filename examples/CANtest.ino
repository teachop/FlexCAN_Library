// -------------------------------------------------------------
// CANtest for Teensy 3.1
// by teachop
//

#include <FlexCAN.h>

int led = 13;
FlexCAN CANbus(125000);
static CAN_message msg;


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
  digitalWrite(led, HIGH);
  delay(500);
  digitalWrite(led, LOW);
  delay(500);
  for( int loop=0; loop<8; ++loop ) {
    msg.buf[loop] = 'A'+loop;
  }
  msg.len = 8;
  msg.id = 0x555;
  CANbus.send(&msg);
}

