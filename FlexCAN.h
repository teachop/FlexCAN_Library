
#include <Arduino.h>

typedef struct CAN_message
{
  uint32_t id;
  uint16_t timeout;
  uint8_t rtr;
  uint8_t len;
  uint8_t buf[8];
} CAN_message;


// -------------------------------------------------------------
class FlexCAN {
public:
  FlexCAN(uint32_t baud = 125000);
  ~FlexCAN(void);
  void begin(void);
  int send(CAN_message *msg);
  int recv(CAN_message *msg);

private:
};
