A simple-to-use Arduino library for CANbus on Teensy 3.1

Status: Working.  New features: Extended identifiers and receive filtering have just been contributed, please help test and give feedback.


FlexCAN is a driver for the CAN0 peripheral built into the Teensy 3.1 CPU.  

When the FlexCAN object is constructed, Arduino pins Digital 3 and Digital 4 (module connections 5 and 6) are assigned to CAN functions.  These should be wired to a 3.3V CAN transceiver TXD and RXD respectively.

5V transceivers may also be an option if the system has regulated +5V available - the Teensy CPU is 5V tolerant and most 5V transceivers will accept a 3V TXD.  This is a good option for breadboarding due to available thru-hole 5V parts.

Note that CAN will normally not work without termination resistors.

Supported baud rates are 125000, 250000, 500000, and 1000000 bits per second.  If the baud rate is not specified it will default to 125000.

begin()
Enable the CAN to start actively participating on the CANbus.

end()
Disable the CAN from participating on the CANbus.  Pins remain assigned to the alternate function CAN0.

write(message)
Send a frame of 1 to 8 bytes using the given identifier.  write() will return 0 if no buffer was available for sending (see "caller blocking" in the wiki).

message is a CAN_message_t type buffer structure.

read(message)
Receive a frame into "message" if available.  read() will return 1 if a frame was copied into the callers buffer, or 0 if no frame is available (see "caller blocking" in the wiki).

available()
Returns 1 if at least one receive frame is waiting, or 0 if no frame is available.

Use of optional RX filtering:

begin(mask)
Enable the CAN to start actively participating on the CANbus.  Enable reception of all messages that fit the mask.  This is a global mask that applies to all the receive filters.

setFilter(filter, number)
Set the receive filter selected by number, 0-7.  When using filters it is required to set them all. If the application uses less than 8 filters, duplicate one filter for the unused ones.


