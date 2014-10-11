// -------------------------------------------------------------
// a simple Arduino Teensy3.1 CAN driver
// by teachop
//
#include "FlexCAN.h"
#include "kinetis_flexcan.h"

static const int txb = 8; // with default settings, all buffers before this are consumed by the FIFO
static const int txBuffers = 8;
static const int rxb = 0;

// -------------------------------------------------------------
FlexCAN::FlexCAN(uint32_t baud)
{
  // set up the pins, 3=PTA12=CAN0_TX, 4=PTA13=CAN0_RX
  CORE_PIN3_CONFIG = PORT_PCR_MUX(2);
  CORE_PIN4_CONFIG = PORT_PCR_MUX(2);// | PORT_PCR_PE | PORT_PCR_PS;
  // select clock source 16MHz xtal
  OSC0_CR |= OSC_ERCLKEN;
  SIM_SCGC6 |=  SIM_SCGC6_FLEXCAN0;
  FLEXCAN0_CTRL1 &= ~FLEXCAN_CTRL_CLK_SRC;

  // enable CAN
  FLEXCAN0_MCR |=  FLEXCAN_MCR_FRZ;
  FLEXCAN0_MCR &= ~FLEXCAN_MCR_MDIS;
  while(FLEXCAN0_MCR & FLEXCAN_MCR_LPM_ACK)
    ;
  // soft reset
  FLEXCAN0_MCR ^=  FLEXCAN_MCR_SOFT_RST;
  while(FLEXCAN0_MCR & FLEXCAN_MCR_SOFT_RST)
    ;
  // wait for freeze ack
  while(!(FLEXCAN0_MCR & FLEXCAN_MCR_FRZ_ACK))
    ;
  // disable self-reception
  FLEXCAN0_MCR |= FLEXCAN_MCR_SRX_DIS;

  //enable RX FIFO
  FLEXCAN0_MCR |= FLEXCAN_MCR_FEN;

  // segment splits and clock divisor based on baud rate
  if ( 50000 == baud ) {
    FLEXCAN0_CTRL1 = (FLEXCAN_CTRL_PROPSEG(2) | FLEXCAN_CTRL_RJW(1)
                      | FLEXCAN_CTRL_PSEG1(7) | FLEXCAN_CTRL_PSEG2(3) | FLEXCAN_CTRL_PRESDIV(19));
  } else if ( 100000 == baud ) {
    FLEXCAN0_CTRL1 = (FLEXCAN_CTRL_PROPSEG(2) | FLEXCAN_CTRL_RJW(1)
                      | FLEXCAN_CTRL_PSEG1(7) | FLEXCAN_CTRL_PSEG2(3) | FLEXCAN_CTRL_PRESDIV(9));
  } else if ( 250000 == baud ) {
    FLEXCAN0_CTRL1 = (FLEXCAN_CTRL_PROPSEG(2) | FLEXCAN_CTRL_RJW(1)
                      | FLEXCAN_CTRL_PSEG1(7) | FLEXCAN_CTRL_PSEG2(3) | FLEXCAN_CTRL_PRESDIV(3));
  } else if ( 500000 == baud ) {
    FLEXCAN0_CTRL1 = (FLEXCAN_CTRL_PROPSEG(2) | FLEXCAN_CTRL_RJW(1)
                      | FLEXCAN_CTRL_PSEG1(7) | FLEXCAN_CTRL_PSEG2(3) | FLEXCAN_CTRL_PRESDIV(1));
  } else if ( 1000000 == baud ) {
    FLEXCAN0_CTRL1 = (FLEXCAN_CTRL_PROPSEG(2) | FLEXCAN_CTRL_RJW(0)
                      | FLEXCAN_CTRL_PSEG1(1) | FLEXCAN_CTRL_PSEG2(1) | FLEXCAN_CTRL_PRESDIV(1));
  } else { // 125000
    FLEXCAN0_CTRL1 = (FLEXCAN_CTRL_PROPSEG(2) | FLEXCAN_CTRL_RJW(1)
                      | FLEXCAN_CTRL_PSEG1(7) | FLEXCAN_CTRL_PSEG2(3) | FLEXCAN_CTRL_PRESDIV(7));
  }

  // Default mask is allow everything
  defaultMask.rtr = 0;
  defaultMask.ext = 0;
  defaultMask.id = 0;
}


// -------------------------------------------------------------
void FlexCAN::end(void)
{
  // enter freeze mode
  FLEXCAN0_MCR |= (FLEXCAN_MCR_HALT);
  while(!(FLEXCAN0_MCR & FLEXCAN_MCR_FRZ_ACK))
    ;
}


// -------------------------------------------------------------
void FlexCAN::begin(const CAN_filter_t &mask)
{
  FLEXCAN0_RXMGMASK = 0;

  //enable reception of all messages that fit the mask
  if (mask.ext) {
    FLEXCAN0_RXFGMASK = ((mask.rtr?1:0) << 31) | ((mask.ext?1:0) << 30) | ((mask.id & FLEXCAN_MB_ID_EXT_MASK) << 1);
  } else {
    FLEXCAN0_RXFGMASK = ((mask.rtr?1:0) << 31) | ((mask.ext?1:0) << 30) | (FLEXCAN_MB_ID_IDSTD(mask.id) << 1);
  }

  // start the CAN
  FLEXCAN0_MCR &= ~(FLEXCAN_MCR_HALT);
  // wait till exit of freeze mode
  while(FLEXCAN0_MCR & FLEXCAN_MCR_FRZ_ACK);

  // wait till ready
  while(FLEXCAN0_MCR & FLEXCAN_MCR_NOT_RDY);

  //set tx buffers to inactive
  for (int i = txb; i < txb + txBuffers; i++) {
    FLEXCAN0_MBn_CS(i) = FLEXCAN_MB_CS_CODE(FLEXCAN_MB_CODE_TX_INACTIVE);
  }
}


// -------------------------------------------------------------
void FlexCAN::setFilter(const CAN_filter_t &filter, uint8_t n)
{
  if ( 8 > n ) {
    if (filter.ext) {
      FLEXCAN0_IDFLT_TAB(n) = ((filter.rtr?1:0) << 31) | ((filter.ext?1:0) << 30) | ((filter.id & FLEXCAN_MB_ID_EXT_MASK) << 1);
    } else {
      FLEXCAN0_IDFLT_TAB(n) = ((filter.rtr?1:0) << 31) | ((filter.ext?1:0) << 30) | (FLEXCAN_MB_ID_IDSTD(filter.id) << 1);
    }
  }
}


// -------------------------------------------------------------
int FlexCAN::available(void)
{
  //In FIFO mode, the following interrupt flag signals availability of a frame
  return (FLEXCAN0_IFLAG1 & FLEXCAN_IMASK1_BUF5M)? 1:0;
}


// -------------------------------------------------------------
int FlexCAN::read(CAN_message_t &msg)
{
  unsigned long int startMillis;

  startMillis = msg.timeout? millis() : 0;

  while( !available() ) {
    if ( !msg.timeout || (msg.timeout<=(millis()-startMillis)) ) {
      // early EXIT nothing here
      return 0;
    }
    yield();
  }

  // get identifier and dlc
  msg.len = FLEXCAN_get_length(FLEXCAN0_MBn_CS(rxb));
  msg.ext = (FLEXCAN0_MBn_CS(rxb) & FLEXCAN_MB_CS_IDE)? 1:0;
  msg.id  = (FLEXCAN0_MBn_ID(rxb) & FLEXCAN_MB_ID_EXT_MASK);
  if(!msg.ext) {
    msg.id >>= FLEXCAN_MB_ID_STD_BIT_NO;
  }

  // copy out message
  uint32_t dataIn = FLEXCAN0_MBn_WORD0(rxb);
  msg.buf[3] = dataIn;
  dataIn >>=8;
  msg.buf[2] = dataIn;
  dataIn >>=8;
  msg.buf[1] = dataIn;
  dataIn >>=8;
  msg.buf[0] = dataIn;
  if ( 4 < msg.len ) {
    dataIn = FLEXCAN0_MBn_WORD1(rxb);
    msg.buf[7] = dataIn;
    dataIn >>=8;
    msg.buf[6] = dataIn;
    dataIn >>=8;
    msg.buf[5] = dataIn;
    dataIn >>=8;
    msg.buf[4] = dataIn;
  }
  for( int loop=msg.len; loop<8; ++loop ) {
    msg.buf[loop] = 0;
  }

  //notify FIFO that message has been read
  FLEXCAN0_IFLAG1 = FLEXCAN_IMASK1_BUF5M;

  return 1;
}


// -------------------------------------------------------------
int FlexCAN::write(const CAN_message_t &msg)
{
  unsigned long int startMillis;

  startMillis = msg.timeout? millis() : 0;

  // find an available buffer
  int buffer = -1;
  for ( int index = txb; ; ) {
    if ((FLEXCAN0_MBn_CS(index) & FLEXCAN_MB_CS_CODE_MASK) == FLEXCAN_MB_CS_CODE(FLEXCAN_MB_CODE_TX_INACTIVE)) {
      buffer = index;
      break;// found one
    }
    if ( !msg.timeout ) {
      if ( ++index >= (txb+txBuffers) ) {
        return 0;// early EXIT no buffers available
      }
    } else {
      // blocking mode, only 1 txb used to guarantee frames in order
      if ( msg.timeout <= (millis()-startMillis) ) {
        return 0;// timed out
      }
      yield();
    }
  }

  // transmit the frame
  FLEXCAN0_MBn_CS(buffer) = FLEXCAN_MB_CS_CODE(FLEXCAN_MB_CODE_TX_INACTIVE);
  if(msg.ext) {
    FLEXCAN0_MBn_ID(buffer) = (msg.id & FLEXCAN_MB_ID_EXT_MASK);
  } else {
    FLEXCAN0_MBn_ID(buffer) = FLEXCAN_MB_ID_IDSTD(msg.id);
  }
  FLEXCAN0_MBn_WORD0(buffer) = (msg.buf[0]<<24)|(msg.buf[1]<<16)|(msg.buf[2]<<8)|msg.buf[3];
  FLEXCAN0_MBn_WORD1(buffer) = (msg.buf[4]<<24)|(msg.buf[5]<<16)|(msg.buf[6]<<8)|msg.buf[7];
  if(msg.ext) {
    FLEXCAN0_MBn_CS(buffer) = FLEXCAN_MB_CS_CODE(FLEXCAN_MB_CODE_TX_ONCE)
                              | FLEXCAN_MB_CS_LENGTH(msg.len) | FLEXCAN_MB_CS_SRR | FLEXCAN_MB_CS_IDE;
  } else {
    FLEXCAN0_MBn_CS(buffer) = FLEXCAN_MB_CS_CODE(FLEXCAN_MB_CODE_TX_ONCE)
                              | FLEXCAN_MB_CS_LENGTH(msg.len);
  }

  return 1;
}

