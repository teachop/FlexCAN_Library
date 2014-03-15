A simple-to-use Arduino library for CANbus on Teensy 3.1

Current status == under construction, operating.
Goals: Stable, low resource usage, Keep It Simple.

TODO:  Caller blocking (see wiki proposal).  Optional rx filtering.

FlexCAN is a driver for the CAN0 peripheral built into the Teensy 3.1 CPU.  When the FlexCAN object is constructed the Arduino pins Digital 3 and Digital 4 (module connections 5 and 6) are assigned to CAN functions.  These should be wired to a 3.3V CAN transceiver TXD and RXD respectively.  Note that CAN will normally not work without termination resistors.  Supported baud rates are 125000, 250000, 500000, and 1000000 bits per second.  If the baud rate is not specified it will default to 125000.

begin()
Enable the CAN to start actively participating on the CANbus.

end()
Disable the CAN from participating on the CANbus.  Pins remain assigned to the alternate function CAN0.

write(message)
Send a frame of 1 to 8 bytes using the given identifier.  write() will return 0 if no buffer was available for sending.

read(message)
Receive a frame into "message" if available.  read() will return 1 if a frame was copied into the callers buffer, or 0 if no frame is available.

available()
Returns 1 if at least one receive frame is waiting, or 0 if no frame is available.

message is a CAN_message_t type buffer structure.
