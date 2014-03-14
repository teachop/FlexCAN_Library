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
  // select clock source
  SIM_SCGC6 |=  SIM_SCGC6_FLEXCAN0;
  FLEXCAN0_CTRL1 |= FLEXCAN_CTRL_CLK_SRC;

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

  // segment timings from freescale loopback test
  if ( 250000 == baud ) {
    FLEXCAN0_CTRL1 = (FLEXCAN_CTRL_PROPSEG(2) | FLEXCAN_CTRL_RJW(1)
                    | FLEXCAN_CTRL_PSEG1(3) | FLEXCAN_CTRL_PSEG2(3) | FLEXCAN_CTRL_PRESDIV(15));
  } else if ( 500000 == baud ) {
    FLEXCAN0_CTRL1 = (FLEXCAN_CTRL_PROPSEG(2) | FLEXCAN_CTRL_RJW(1)
                    | FLEXCAN_CTRL_PSEG1(3) | FLEXCAN_CTRL_PSEG2(3) | FLEXCAN_CTRL_PRESDIV(7));
  } else if ( 1000000 == baud ) {
    FLEXCAN0_CTRL1 = (FLEXCAN_CTRL_PROPSEG(3) | FLEXCAN_CTRL_RJW(0)
                    | FLEXCAN_CTRL_PSEG1(0) | FLEXCAN_CTRL_PSEG2(1) | FLEXCAN_CTRL_PRESDIV(5));
  } else { // 125000
    FLEXCAN0_CTRL1 = (FLEXCAN_CTRL_PROPSEG(2) | FLEXCAN_CTRL_RJW(2)
                    | FLEXCAN_CTRL_PSEG1(3) | FLEXCAN_CTRL_PSEG2(3) | FLEXCAN_CTRL_PRESDIV(31));
  }
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
void FlexCAN::begin(void)
{
  FLEXCAN0_RXMGMASK = 0;

  //enable reception of all messages into the FIFO (filters ignored)
  FLEXCAN0_RXFGMASK = 0;


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

bool FlexCAN::available(void)
{
	//In FIFO mode, the following interrupt flag signals availability of a frame
	return FLEXCAN0_IFLAG1 & FLEXCAN_IMASK1_BUF5M;
}

// -------------------------------------------------------------
int FlexCAN::read(CAN_message_t &msg)
{
  
  if( !available() ) {
    // early EXIT nothing here
    return 0;
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
  msg.buf[3] = dataIn; dataIn >>=8;
  msg.buf[2] = dataIn; dataIn >>=8;
  msg.buf[1] = dataIn; dataIn >>=8;
  msg.buf[0] = dataIn;
  if ( 4 < msg.len ) {
    dataIn = FLEXCAN0_MBn_WORD1(rxb);
    msg.buf[7] = dataIn; dataIn >>=8;
    msg.buf[6] = dataIn; dataIn >>=8;
    msg.buf[5] = dataIn; dataIn >>=8;
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
  //find an available buffer
  int buffer = -1;
  for (int i = txb; i < txb + txBuffers; i++) {
    if ((FLEXCAN0_MBn_CS(i) & FLEXCAN_MB_CS_CODE_MASK) == FLEXCAN_MB_CS_CODE(FLEXCAN_MB_CODE_TX_INACTIVE)) {
      buffer = i;
	  break;
    }
  }

  //No buffers available
  if (buffer < 0)
	  return 0;

  FLEXCAN0_MBn_CS(buffer) = FLEXCAN_MB_CS_CODE(FLEXCAN_MB_CODE_TX_INACTIVE);
  FLEXCAN0_MBn_ID(buffer) = FLEXCAN_MB_ID_IDSTD(msg.id);
  FLEXCAN0_MBn_WORD0(buffer) = (msg.buf[0]<<24)|(msg.buf[1]<<16)|(msg.buf[2]<<8)|msg.buf[3];
  FLEXCAN0_MBn_WORD1(buffer) = (msg.buf[4]<<24)|(msg.buf[5]<<16)|(msg.buf[6]<<8)|msg.buf[7];  
  FLEXCAN0_MBn_CS(buffer) = FLEXCAN_MB_CS_CODE(FLEXCAN_MB_CODE_TX_ONCE)
                      | FLEXCAN_MB_CS_LENGTH(msg.len);

  return 1;
}

