// -------------------------------------------------------------
// a very simple Teensy3.1 CAN
// by teachop
//

#include "FlexCAN.h"
#include "kinetis_flexcan.h"


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
  // initialize message buffers
  for( int loop=0; loop<16; ++loop ) {
    FLEXCAN0_MBn_CS(loop) = 0;
    FLEXCAN0_MBn_ID(loop) = 0;
    FLEXCAN0_MBn_WORD0(loop) = 0;
    FLEXCAN0_MBn_WORD1(loop) = 0;
  }
  // TODO set the baud rate
  FLEXCAN0_CTRL1 = (0 | FLEXCAN_CTRL_PROPSEG(2) | FLEXCAN_CTRL_RJW(2)
                      | FLEXCAN_CTRL_PSEG1(3) | FLEXCAN_CTRL_PSEG2(3)
                      | FLEXCAN_CTRL_PRESDIV(31));
}


// -------------------------------------------------------------
FlexCAN::~FlexCAN(void)
{
  // enter freeze mode
  FLEXCAN0_MCR |= (FLEXCAN_MCR_HALT);
  while(!(FLEXCAN0_MCR & FLEXCAN_MCR_FRZ_ACK))
    ;
}


// -------------------------------------------------------------
void FlexCAN::begin(void)
{
  // start the CAN
  FLEXCAN0_MCR &= ~(FLEXCAN_MCR_HALT);
  // wait till exit of freeze mode
  while(FLEXCAN0_MCR & FLEXCAN_MCR_FRZ_ACK)
    ;
  // wait till ready 
  while(FLEXCAN0_MCR & FLEXCAN_MCR_NOT_RDY)
    ;
}


// -------------------------------------------------------------
int FlexCAN::recv(CAN_message *msg)
{
}


// -------------------------------------------------------------
int FlexCAN::send(CAN_message *msg)
{
    FLEXCAN0_MBn_CS(0) = FLEXCAN_MB_CS_CODE(FLEXCAN_MB_CODE_TX_INACTIVE);
    FLEXCAN0_MBn_ID(0) = FLEXCAN_MB_ID_IDSTD(msg->id);
    FLEXCAN0_MBn_WORD0(0) = (msg->buf[0]<<24)|(msg->buf[1]<<16)|(msg->buf[2]<<8)|msg->buf[3];
    FLEXCAN0_MBn_WORD1(0) = (msg->buf[4]<<24)|(msg->buf[5]<<16)|(msg->buf[6]<<8)|msg->buf[7];  
    FLEXCAN0_MBn_CS(0) = FLEXCAN_MB_CS_CODE(FLEXCAN_MB_CODE_TX_ONCE)
                       | FLEXCAN_MB_CS_LENGTH(msg->len);     
}







