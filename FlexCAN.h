// -------------------------------------------------------------
// a simple Arduino Teensy3.1 CAN driver
// by teachop
//
#ifndef __FLEXCAN_H__
#define __FLEXCAN_H__

#include <Arduino.h>

typedef struct CAN_message_t {
  uint32_t id; // can identifier
  uint8_t ext; // identifier is extended
  uint8_t len; // length of data
  uint16_t timeout; // milliseconds, zero will disable waiting
  uint8_t buf[8];
} CAN_message_t;

typedef struct CAN_filter_t {
  uint8_t rtr;
  uint8_t ext;
  uint32_t id;
} CAN_filter_t;

// -------------------------------------------------------------
class FlexCAN
{
private:
  struct CAN_filter_t defaultMask;

public:
  FlexCAN(uint32_t baud = 125000);
  void begin(const CAN_filter_t &mask);
  inline void begin()
  {
    begin(defaultMask);
  }
  void setFilter(const CAN_filter_t &filter, uint8_t n);
  void end(void);
  int available(void);
  int write(const CAN_message_t &msg);
  int read(CAN_message_t &msg);

};

#endif // __FLEXCAN_H__
