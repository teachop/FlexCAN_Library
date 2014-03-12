// -------------------------------------------------------------
// a simple Arduino Teensy3.1 CAN driver
// by teachop
//
#include <Arduino.h>

typedef struct CAN_message
{
  uint32_t id; // can identifier
  uint8_t ext; // identifier is extended
  uint8_t len; // length of data
  uint16_t timeout;
  uint8_t buf[8];
} CAN_message;


// -------------------------------------------------------------
class FlexCAN {
public:
  FlexCAN(uint32_t baud = 125000);
  void begin(void);
  void end(void);
  bool available(void);
  int write(const CAN_message &msg);
  int  read(CAN_message &msg);

private:
};
