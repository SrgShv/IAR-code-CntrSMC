/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file    spi.c
  * @brief   This file provides code for the configuration
  *          of the SPI instances.
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2024 STMicroelectronics.
  * All rights reserved.
  *
  * This software is licensed under terms that can be found in the LICENSE file
  * in the root directory of this software component.
  * If no LICENSE file comes with this software, it is provided AS-IS.
  *
  ******************************************************************************
  */

/* Includes ------------------------------------------------------------------*/
#include "spi.h"
#include "stdio.h"
#include "UDP.h"

//#define RX_BUFF_SZ      1600
#define TRUE            1
#define FALSE           0

SPI_HandleTypeDef hspi3;
DMA_HandleTypeDef hdma_spi3_rx;
DMA_HandleTypeDef hdma_spi3_tx;

/** ENC28J60 Revision ID: 06 **/
//static uint8_t buffer[ENC28J60_BUFSIZE];

extern "C"
{
   void enableExtiIRQ_A(bool flg);
   void setSelLanA(uint8_t flg);
   void setRstLanA(uint8_t flg);
}
extern void SystemReset(void);
extern void onStartTimer4(uint32_t time);
extern CBuffLAN *pBuffRxLAN;
extern CBuffLAN *pBuffTxLAN;
extern dPTR m_dPtr;
extern dPTR m_dPtrRx;
extern dPTR m_dPtrTx;
extern CEthernet *pEthernet;

//extern void setOnIRQ(bool flg);
extern void mainRunTime();
extern void mainTickRunTime();
//extern void HandleLanSockets(uint8_t *data, uint16_t len);

static uint16_t gNextPacketPtr = ENC28J60_RXSTART;

static volatile uint8_t stepLan_1 = 0;
static volatile uint8_t stepLan_2 = 0;
static volatile uint8_t stepLan_3 = 0;
static volatile uint8_t stepLan_4 = 0;

static volatile uint32_t timeLan_1 = 0;
static volatile uint32_t timeLan_2 = 0;
static volatile uint32_t timeLan_3 = 0;
static volatile uint32_t timeLan_4 = 0;
static volatile bool RxFlgLAN = false;

//static bool     unreleasedPacket = false;

void delay_us(uint32_t us)
{
   uint32_t tcnt = 0;
   while(++tcnt < us*5) ;
}

void setStep(uint8_t stepNo, uint8_t pos)
{
   switch(stepNo)    /** RX DMA-SPI-LAN **/
   {
      case 1:
         stepLan_1 = pos;
         break;
      case 2:
         stepLan_2 = pos;
         break;
      case 3:
         stepLan_3 = pos;
         break;
      case 4:
         stepLan_4 = pos;
         break;
      case 5:
         break;
      case 6:
         break;
      default:
         break;
   };
}

/** After IRQ DMA TxRx complete => **/
void mainRunTime()
{
   uint8_t data1 = 0;
   if(RxFlgLAN)
   {
      __disable_irq();
      RxFlgLAN = false;
      __enable_irq();

      pEthernet->m_pLanA->enc28j60_set_bank(ECON2);
      pEthernet->m_pLanA->onSetSelect();
      data1 = (ENC28J60_SPI_BFS | (ECON2 & ENC28J60_ADDR_MASK));
      HAL_SPI_Transmit(&hspi3, &data1, 1, 1);
      data1 = ECON2_PKTDEC;
      HAL_SPI_Transmit(&hspi3, &data1, 1, 1);
      delay_us(1);
      pEthernet->m_pLanA->onClrSelect();

      uint8_t *pDat = 0;
      uint16_t *pLen = 0;
      if(pBuffRxLAN->onCheck())
      {
         if(pBuffRxLAN->onGetReadBuff(m_dPtr))
         {
            gNextPacketPtr = *(uint16_t *)(&m_dPtr.data[1]);
            pDat = (uint8_t *)(&m_dPtr.data[7]);
            pLen = (uint16_t *)(&m_dPtr.data[3]);
            HandleLanSockets(pDat, (*pLen - 4));
         };
      };
   };
};

void mainTickRunTime() // 1 msec
{
};

/* SPI3 init function */
void MX_SPI3_Init(void)
{
  hspi3.Instance = SPI3;
  hspi3.Init.Mode = SPI_MODE_MASTER;
  hspi3.Init.Direction = SPI_DIRECTION_2LINES;
  hspi3.Init.DataSize = SPI_DATASIZE_8BIT;
  hspi3.Init.CLKPolarity = SPI_POLARITY_LOW;
  hspi3.Init.CLKPhase = SPI_PHASE_1EDGE;
  hspi3.Init.NSS = SPI_NSS_SOFT;
  hspi3.Init.BaudRatePrescaler = SPI_BAUDRATEPRESCALER_4;
  hspi3.Init.FirstBit = SPI_FIRSTBIT_MSB;
  hspi3.Init.TIMode = SPI_TIMODE_DISABLE;
  hspi3.Init.CRCCalculation = SPI_CRCCALCULATION_DISABLE;
  hspi3.Init.CRCPolynomial = 10;
  if (HAL_SPI_Init(&hspi3) != HAL_OK)
  {
    _Error_Handler((char *)__FILE__, __LINE__);
  }

}

//#define INIT_DMA_SPI3_SWITCH
void HAL_SPI_MspInit(SPI_HandleTypeDef* spiHandle)
{

   GPIO_InitTypeDef GPIO_InitStruct = {0};
   if(spiHandle->Instance==SPI3)
   {
      /* SPI3 clock enable */
      __HAL_RCC_SPI3_CLK_ENABLE();
      __HAL_RCC_DMA1_CLK_ENABLE();
      __HAL_RCC_GPIOC_CLK_ENABLE();
      /**SPI3 GPIO Configuration
      PC10     ------> SPI3_SCK
      PC11     ------> SPI3_MISO
      PC12     ------> SPI3_MOSI
      */
      GPIO_InitStruct.Pin = GPIO_PIN_10|GPIO_PIN_11|GPIO_PIN_12;
      GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
      GPIO_InitStruct.Pull = GPIO_NOPULL;
      GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_VERY_HIGH;
      GPIO_InitStruct.Alternate = GPIO_AF6_SPI3;
      HAL_GPIO_Init(GPIOC, &GPIO_InitStruct);

      /* SPI3 DMA Init */
      /* SPI3_RX Init */
      hdma_spi3_rx.Instance = DMA1_Stream0;
      hdma_spi3_rx.Init.Channel = DMA_CHANNEL_0;
      hdma_spi3_rx.Init.Direction = DMA_PERIPH_TO_MEMORY;
      hdma_spi3_rx.Init.PeriphInc = DMA_PINC_DISABLE;
      hdma_spi3_rx.Init.MemInc = DMA_MINC_ENABLE;
      hdma_spi3_rx.Init.PeriphDataAlignment = DMA_PDATAALIGN_BYTE;
      hdma_spi3_rx.Init.MemDataAlignment = DMA_MDATAALIGN_BYTE;
      hdma_spi3_rx.Init.Mode = DMA_NORMAL;
      hdma_spi3_rx.Init.Priority = DMA_PRIORITY_VERY_HIGH;
      hdma_spi3_rx.Init.FIFOMode = DMA_FIFOMODE_DISABLE;
      if (HAL_DMA_Init(&hdma_spi3_rx) != HAL_OK)
      {
         _Error_Handler((char *)__FILE__, __LINE__);
      }

      __HAL_LINKDMA(spiHandle,hdmarx,hdma_spi3_rx);

      /* DMA1_Stream0_IRQn interrupt configuration */
      HAL_NVIC_SetPriority(DMA1_Stream0_IRQn, 3, 2);
      HAL_NVIC_EnableIRQ(DMA1_Stream0_IRQn);


      /* SPI3_TX Init */
      hdma_spi3_tx.Instance = DMA1_Stream7;
      hdma_spi3_tx.Init.Channel = DMA_CHANNEL_0;
      hdma_spi3_tx.Init.Direction = DMA_MEMORY_TO_PERIPH;
      hdma_spi3_tx.Init.PeriphInc = DMA_PINC_DISABLE;
      hdma_spi3_tx.Init.MemInc = DMA_MINC_ENABLE;
      hdma_spi3_tx.Init.PeriphDataAlignment = DMA_PDATAALIGN_BYTE;
      hdma_spi3_tx.Init.MemDataAlignment = DMA_MDATAALIGN_BYTE;
      hdma_spi3_tx.Init.Mode = DMA_NORMAL; //DMA_CIRCULAR
      hdma_spi3_tx.Init.Priority = DMA_PRIORITY_VERY_HIGH;
      hdma_spi3_tx.Init.FIFOMode = DMA_FIFOMODE_DISABLE;
      if (HAL_DMA_Init(&hdma_spi3_tx) != HAL_OK)
      {
         _Error_Handler((char *)__FILE__, __LINE__);
      }

      __HAL_LINKDMA(spiHandle,hdmatx,hdma_spi3_tx);

      HAL_NVIC_SetPriority(DMA1_Stream7_IRQn, 3, 1);  // TX SPI3 DMA
      HAL_NVIC_EnableIRQ(DMA1_Stream7_IRQn);
   }
}

void HAL_SPI_MspDeInit(SPI_HandleTypeDef* spiHandle)
{

  if(spiHandle->Instance==SPI3)
  {
  /* USER CODE BEGIN SPI3_MspDeInit 0 */

  /* USER CODE END SPI3_MspDeInit 0 */
    /* Peripheral clock disable */
    __HAL_RCC_SPI3_CLK_DISABLE();

    /**SPI3 GPIO Configuration
    PC10     ------> SPI3_SCK
    PC11     ------> SPI3_MISO
    PC12     ------> SPI3_MOSI
    */
    HAL_GPIO_DeInit(GPIOC, GPIO_PIN_10|GPIO_PIN_11|GPIO_PIN_12);

  /* USER CODE BEGIN SPI3_MspDeInit 1 */

  /* USER CODE END SPI3_MspDeInit 1 */
  }
}
// =========== classes ============

/**************************** ETHERNET *****************************/
CEthernet::CEthernet() :
   m_TxReadyFlgA(false),
   m_pLanA(0)
{
}

CEthernet::~CEthernet()
{
}

void CEthernet::onStartInit(uint8_t *MAC)
{
   m_pLanA = &m_LanA;
   m_pLanA->onInit(MAC);
   m_TxReadyFlgA = true;
}

void CEthernet::onRestart(uint8_t *MAC)
{
   m_pLanA->onInit(MAC);
}

void CEthernet::onClearIrqFlagsLAN(void)
{
   m_pLanA->onClearIrqFlags();
}

/**
nextFlgRX = true;
for(uint8_t i=0; i<10; i++)
{
   if(nextFlgRX)
   {
      if(pBuffLAN->onGetWriteBuff(m_dPtr))
      {
         pEthernet->m_pLanA->enc28j60_recv_packet(m_dPtr.data, m_dPtr.byteCount, nextFlgRX);
      };
   }
   else break;
};
**/
//static bool nextFlgRX = false;
//void CEthernet::onRunTime(void)
//{
//   printf("start RX => \n\r");
//   pEthernet->m_pLanA->startRX();
//   nextFlgRX = true;
//   uint32_t dCnt = 0;
//   while(nextFlgRX)
//   {
//      pEthernet->m_pLanA->startRX();
//      //pBuffLAN->onGetWriteBuff(m_dPtr);
//      //pEthernet->m_pLanA->enc28j60_recv_packet(m_dPtr.data, m_dPtr.byteCount, nextFlgRX);
//      printf("++Land %d\n\r", dCnt);
//      ++dCnt;
//   };
//}

void CEthernet::onTickRunTime(void)
{
   this->m_pLanA->enc28j60_set_bank(EIR);
   this->m_pLanA->enc28j60_read_op(ENC28J60_SPI_RCR, EIR);
   if(this->m_pLanA->enc28j60_rcr(EPKTCNT) > 0)
   {
      pEthernet->m_pLanA->startRX();
   };

//   switch(stepLan_2)    /** - **/
//   {
//      case 1:
//         break;
//      case 2:
//         break;
//      case 3:
//         break;
//      case 4:
//         break;
//      case 5:
//         break;
//      case 6:
//         break;
//      default:
//         break;
//   };
}

bool CEthernet::onSend(uint8_t *data, uint16_t len)
{
   bool res = false;
   if(m_TxReadyFlgA == true)
   {
      m_TxReadyFlgA = false;
      //enableExtiIRQ_A(false);
      m_pLanA->onSend(data, len);
      //enableExtiIRQ_A(true);
      m_TxReadyFlgA = true;
      res = true;
   };
   return res;
}

bool CEthernet::onRecieve(uint8_t *data, uint16_t* pLen)
{
   //bool res = false;
   uint16_t len = m_pLanA->onRecieve(data);
   if(len > ENC28J60_MAXFRAME) len = ENC28J60_MAXFRAME;
   *pLen = len;
   if(len > 0) return true;
   return false;
}

//bool CEthernet::onRecieveDMA(uint8_t *data, uint16_t* pLen)
//{
//   //bool res = false;
//   uint16_t len = m_pLanA->onRecieve(data);
//   if(len > ENC28J60_MAXFRAME) len = ENC28J60_MAXFRAME;
//   *pLen = len;
//   if(len > 0) return true;
//   return false;
//}



#define ENC28J60_FULL_DUPLEX_SUPPORT
/*************************************CLAN***************************************/
CLAN::CLAN() :
   m_MacFlag(0),
   m_rxBuff(0)
{
   m_rxBuff = new uint8_t [RX_BUFF_SZ];
}

#define CODE_SWITCH_1
void CLAN::onInit(uint8_t *mac)
{
   for(uint8_t i=0; i<6; i++)
   {
      m_macadr[i] = mac[i];
   };
   m_mac = (uint8_t *)&(m_macadr[0]);

   this->onInitSPI(3);
   m_enc28j60_current_bank = 0;
   m_nextReadPtr = ENC28J60_RXSTART;
   this->onClrSelect();
   delay_us(20000);
   this->onSetSelect();
   delay_us(20000);
   enc28j60_tx(ENC28J60_SPI_SC);    // set Soft reset
   delay_us(20000);
   enc28j60_write_op(ENC28J60_SPI_SC, 0, ENC28J60_SPI_SC);

   delay_us(100);

   this->onClrSelect();
   delay_us(100);

   this->onSetSelect();
   delay_us(20000);         // ENC28J60
   this->onSetReset();
   delay_us(50000);
   this->onClrReset();
   delay_us(50000);
   this->onClrSelect();

   delay_us(20000);         // ENC28J60
   this->onSetReset();
   delay_us(50000);
   this->onClrReset();
   delay_us(50000);

   this->enc28j60_wcr(ERXSTL, (ENC28J60_RXSTART)&0xFF);
	this->enc28j60_wcr(ERXSTH, (ENC28J60_RXSTART)>>8);
	// set receive pointer address
	this->enc28j60_wcr(ERXRDPTL, (ENC28J60_RXSTART)&0xFF);
	this->enc28j60_wcr(ERXRDPTH, (ENC28J60_RXSTART)>>8);
	// RX end
	this->enc28j60_wcr(ERXNDL, ENC28J60_RXEND&0xFF);
	this->enc28j60_wcr(ERXNDH, ENC28J60_RXEND>>8);
	// TX start
	this->enc28j60_wcr(ETXSTL, ENC28J60_TXPPCB&0xFF);
	this->enc28j60_wcr(ETXSTH, ENC28J60_TXPPCB>>8);
	// TX end
	this->enc28j60_wcr(ETXNDL, ENC28J60_TXEND&0xFF);
	this->enc28j60_wcr(ETXNDH, ENC28J60_TXEND>>8);

   this->enc28j60_wcr(MAMXFLL, ENC28J60_MAXFRAME&0xFF);
	this->enc28j60_wcr(MAMXFLH, ENC28J60_MAXFRAME>>8);

      // Set filter, packets with local MAC-address will be accepted
   this->enc28j60_wcr(ERXFCON, ERXFCON_UCEN|ERXFCON_CRCEN|ERXFCON_PMEN|ERXFCON_BCEN);
   this->enc28j60_wcr(EPMM0, 0x3f);
   this->enc28j60_wcr(EPMM1, 0x30);
   this->enc28j60_wcr(EPMCSL, 0xf9);
   this->enc28j60_wcr(EPMCSH, 0xf7);

      // Setup MAC
   this->enc28j60_wcr(MACON1, MACON1_TXPAUS|MACON1_RXPAUS|MACON1_MARXEN);  // Enable flow control, Enable MAC Rx
   this->enc28j60_wcr(MACON2, 0x00);                     // Clear reset

#ifdef ENC28J60_FULL_DUPLEX_SUPPORT
   this->enc28j60_wcr(MACON3, MACON3_PADCFG0|MACON3_TXCRCEN|MACON3_FRMLNEN|MACON3_FULDPX);
   this->enc28j60_wcr(MACON4, MACON4_LONGPRE|MACON4_PUREPRE);
#else
   this->enc28j60_wcr(MACON3, MACON3_PADCFG0|MACON3_TXCRCEN|MACON3_FRMLNEN);
   this->enc28j60_wcr(MACON4, MACON4_DEFER);
#endif

   this->enc28j60_wcr(MAIPGL, 0x12);
   this->enc28j60_wcr(MAIPGH, 0x0c);

#ifdef ENC28J60_FULL_DUPLEX_SUPPORT
   this->enc28j60_wcr(MABBIPG, ENC28J60_MABBIPG_DEFAULT_FD);                      // Set inter-frame gap
#else
   this->enc28j60_wcr(MABBIPG, ENC28J60_MABBIPG_DEFAULT_HD);                      // Set inter-frame gap
#endif

      // set maximum frame size for receive RX bytes (1520 BYTES)
   this->enc28j60_wcr(MAMXFLL, ENC28J60_MAXFRAME&0xFF);
   this->enc28j60_wcr(MAMXFLH, ENC28J60_MAXFRAME>>8);

      // Set MAC address
   this->enc28j60_wcr(MAADR5, m_macadr[0]);
   this->enc28j60_wcr(MAADR4, m_macadr[1]);
   this->enc28j60_wcr(MAADR3, m_macadr[2]);
   this->enc28j60_wcr(MAADR2, m_macadr[3]);
   this->enc28j60_wcr(MAADR1, m_macadr[4]);
   this->enc28j60_wcr(MAADR0, m_macadr[5]);

      // Setup PHY
#ifdef ENC28J60_FULL_DUPLEX_SUPPORT
   enc28j60_write_phy(PHCON1, PHCON1_PDPXMD);                                 // Force full-duplex mode
#else
   enc28j60_write_phy(PHCON1, 0x0000);                                        // Force half-duplex mode
#endif

   enc28j60_write_phy(PHCON2, PHCON2_HDLDIS);                                 // Disable loopback

   // clear interrupt flags
   this->enc28j60_wcr(EIR, 0x00);
      // LedA - yellow, LedB - green
   enc28j60_write_phy(PHLCON, PHLCON_LACFG0|PHLCON_LACFG2|PHLCON_LBCFG2|PHLCON_LBCFG1|PHLCON_LBCFG0|PHLCON_LFRQ0|PHLCON_STRCH); // Configure LED ctrl

   this->enc28j60_set_bank(ECON1);
   // Enable Rx packets
   enc28j60_bfs(ECON1, ECON1_RXEN);
   // enable interrupts
   enc28j60_bfs(EIE, EIE_INTIE|EIE_PKTIE);

   uint16_t revID = 0x00FF & enc28j60_rcr16(EREVID);
   //printf("rev ID: %d\r\n", revID);

   enc28j60_wcr(ERDPTL, (0));          // Buffer read pointer L
   enc28j60_wcr(ERDPTH, (0));          // Buffer read pointer H

   m_MacFlag = 1;
   //HAL_Delay(20);
   delay_us(20000);

}

void CLAN::onClearIrqFlags(void)
{
   // clear interrupt flags
   this->enc28j60_wcr(EIR, 0x00);
}

void CLAN::onSend(uint8_t *data, uint16_t len)
{
   this->enc28j60_send_packet(data, len);
}

uint16_t CLAN::onRecieve(uint8_t *data)
{
   bool nextF = false;
   uint16_t len = 0;
   this->enc28j60_recv_packet(data, &len, nextF);
   return len;
}
//*************************************CLAN******************************

//*************************************CSPI******************************
CSPI::CSPI() :
   m_mac(0),
   m_nextReadPtr(0),
   m_enc28j60_current_bank(0),
   lastRDPTR(0)
{
}

CSPI::~CSPI()
{
}

void CSPI::startRX()
{
   enc28j60_set_bank(EIR);
   enc28j60_read_op(ENC28J60_SPI_RCR, EIR);
   if(enc28j60_rcr(EPKTCNT) > 0)
   {
      if(gNextPacketPtr == 0)
      {
         enc28j60_wcr(ERXRDPTL, (uint8_t)(ENC28J60_RXEND));
         enc28j60_wcr(ERXRDPTH, (uint8_t)((ENC28J60_RXEND)>>8));
      }
      else
      {
         enc28j60_wcr(ERXRDPTL, (uint8_t)(gNextPacketPtr-1));
         enc28j60_wcr(ERXRDPTH, (uint8_t)((gNextPacketPtr-1)>>8));
      };

      enc28j60_wcr(ERDPTL, (gNextPacketPtr));         // Buffer read pointer L
      enc28j60_wcr(ERDPTH, (gNextPacketPtr>>8));      // Buffer read pointer H

      pBuffTxLAN->onGetWriteBuff(m_dPtrTx);
      m_dPtrTx.data[0] = ENC28J60_SPI_RBM;            // init Tx data
      m_dPtrTx.byteCount[0] = gm_max_sz_Eth;
      pBuffTxLAN->onGetReadBuff(m_dPtrTx);
      pBuffRxLAN->onGetWriteBuff(m_dPtrRx);
      m_dPtrRx.byteCount[0] = gm_max_sz_Eth;
      busy = true;
      onSetSelect();
      HAL_SPI_TransmitReceive_DMA(&hspi3, m_dPtrTx.data, m_dPtrRx.data, gm_max_sz_Eth);
   };
}

/**
m_dPtrRx.data:

[0]  garbage
[1]  NextPacketPtrL
[2]  NextPacketPtrH
[3]  PacketLengthL
[4]  PacketLengthH
[5]  StatusL
[6]  StatusH
[7]  Payload[0]
[8]  Payload[1]
...
*/

void CSPI::onDmaComplete()
{
   busy = false;
   RxFlgLAN = true;
   onClrSelect();
}

void CSPI::watchdog(uint32_t now)
{
   if (busy && (now - startTime > 5))
   {
      __HAL_SPI_DISABLE(_hspi);
      __HAL_SPI_ENABLE(_hspi);
      busy = false;
   };
}

//*************************************----******************************

void CSPI::onInitSPI(char spi)
{
   if(m_mac != 0)
   {
      MX_SPI3_Init();
   };
}

void CSPI::onSetSelect(void)
{
   setSelLanA(TRUE);
}

void CSPI::onClrSelect(void)
{
   setSelLanA(FALSE);
}

void CSPI::onSetReset(void)
{
   setRstLanA(TRUE);
}

void CSPI::onClrReset(void)
{
   setRstLanA(FALSE);
}

void CSPI::enc28j60_tx(uint8_t data)
{

   HAL_SPI_Transmit(&hspi3, &data, 1, 1);

}

uint8_t CSPI::enc28j60_rx(void)
{
   uint8_t data = 0;
   HAL_SPI_Receive(&hspi3, &data, 1, 1);
   return data;
}

// Generic SPI read command
uint8_t CSPI::enc28j60_read_op(uint8_t cmd, uint8_t adr)
{
	uint8_t data;
	this->onSetSelect();
	//delay_us(3);
	enc28j60_tx(cmd | (adr & ENC28J60_ADDR_MASK));
	if(adr & 0x80) enc28j60_rx();// throw out dummy byte when reading MII/MAC register
	data = enc28j60_rx();
	this->onClrSelect();
	return data;
}

// Generic SPI write command
void CSPI::enc28j60_write_op(uint8_t cmd, uint8_t adr, uint8_t data)
{
   uint8_t dataT = cmd | (adr & ENC28J60_ADDR_MASK);
	this->onSetSelect();
	//delay_us(3);
	//enc28j60_tx(cmd | (adr & ENC28J60_ADDR_MASK));
   HAL_SPI_Transmit(&hspi3, &dataT, 1, 1);
	//enc28j60_tx(data);
   dataT = data;
   HAL_SPI_Transmit(&hspi3, &dataT, 1, 1);
   delay_us(1);
	this->onClrSelect();
}

// Set register bank
void CSPI::enc28j60_set_bank(uint8_t adr)
{
	uint8_t bank;
	if( (adr & ENC28J60_ADDR_MASK) < ENC28J60_COMMON_CR )
	{
		bank = (adr >> 5) & 0x03; //BSEL1|BSEL0=0x03
		if(bank != m_enc28j60_current_bank)
		{
			enc28j60_write_op(ENC28J60_SPI_BFC, ECON1, 0x03);
			enc28j60_write_op(ENC28J60_SPI_BFS, ECON1, bank);
			m_enc28j60_current_bank = bank;
		};
	};
}

// Read register
uint8_t CSPI::enc28j60_rcr(uint8_t adr)
{
	enc28j60_set_bank(adr);
	return enc28j60_read_op(ENC28J60_SPI_RCR, adr);
}

// Read register pair
uint16_t CSPI::enc28j60_rcr16(uint8_t adr)
{
	enc28j60_set_bank(adr);
	uint16_t res0 = (uint16_t)enc28j60_read_op(ENC28J60_SPI_RCR, adr);
	uint16_t res1 = (uint16_t)enc28j60_read_op(ENC28J60_SPI_RCR, adr);
	return (res0 | (res1 << 8));
}

void CSPI::enc28j60_clrRxPackCount(void)
{
   enc28j60_wcr(EPKTCNT, 0);
   //gNextPacketPtr = ENC28J60_RXSTART;
   //unreleasedPacket = false;
}

uint16_t CSPI::ENC28J60_GetReceivedPacketLength()
{
    uint8_t header[6];  //
    uint16_t packetLength = 0;

    //  ENC28J60 (NSS = LOW)
   this->onSetSelect();

    //  READ BUFFER MEMORY (0x3A)
    uint8_t command = 0x3A;
    HAL_SPI_Transmit(&hspi3, &command, 1, 1);

    //
    HAL_SPI_Receive(&hspi3, header, 6, 3);

    //  ENC28J60 (NSS = HIGH)
    this->onClrSelect();

    //
    packetLength = (header[2] | (header[3] << 8));

    //
    if (packetLength > 4)
        packetLength -= 4;

    return packetLength;
}

void CSPI::enc28j60_prnt8(uint8_t adr)
{
   enc28j60_set_bank(adr);
	uint8_t res = enc28j60_read_op(ENC28J60_SPI_RCR, adr);

   printf("a:0x%02X v:0x%02X, bin: ", adr, res);
   uint8_t d = 0;

   d = 0x01 & (res>>7);
   printf("%d", d);
   d = 0x01 & (res>>6);
   printf("%d", d);
   d = 0x01 & (res>>5);
   printf("%d", d);
   d = 0x01 & (res>>4);
   printf("%d", d);
   printf(" ");
   d = 0x01 & (res>>3);
   printf("%d", d);
   d = 0x01 & (res>>2);
   printf("%d", d);
   d = 0x01 & (res>>1);
   printf("%d", d);
   d = 0x01 & (res);
   printf("%d", d);
   printf("\r\n");
}

void CSPI::enc28j60_prnt16(uint8_t adr)
{
   enc28j60_set_bank(adr);
	uint16_t res0 = (uint16_t)enc28j60_read_op(ENC28J60_SPI_RCR, adr);
	uint16_t res1 = (uint16_t)enc28j60_read_op(ENC28J60_SPI_RCR, adr);
   uint16_t res = (res0 | (res1 << 8));

   printf("a:0x%02X v:0x%04X, bin: ", adr, res);
   uint8_t d = 0;
   d = 0x01 & (res>>15);
   printf("%d", d);
   d = 0x01 & (res>>14);
   printf("%d", d);
   d = 0x01 & (res>>13);
   printf("%d", d);
   d = 0x01 & (res>>12);
   printf("%d", d);
   printf(" ");
   d = 0x01 & (res>>11);
   printf("%d", d);
   d = 0x01 & (res>>10);
   printf("%d", d);
   d = 0x01 & (res>>9);
   printf("%d", d);
   d = 0x01 & (res>>8);
   printf("%d", d);
   printf("  ");

   d = 0x01 & (res>>7);
   printf("%d", d);
   d = 0x01 & (res>>6);
   printf("%d", d);
   d = 0x01 & (res>>5);
   printf("%d", d);
   d = 0x01 & (res>>4);
   printf("%d", d);
   printf(" ");
   d = 0x01 & (res>>3);
   printf("%d", d);
   d = 0x01 & (res>>2);
   printf("%d", d);
   d = 0x01 & (res>>1);
   printf("%d", d);
   d = 0x01 & (res);
   printf("%d", d);
   printf("\r\n");
}

void CSPI::enc28j60_decrCounter(void)
{
//   enc28j60_set_bank(ECON2);
//   enc28j60_bfs(ECON2, ECON2_PKTDEC);

   uint8_t data1 = 0;
   enc28j60_set_bank(ECON2);
   delay_us(1);
   this->onSetSelect();
   data1 = (ENC28J60_SPI_BFS | (ECON2 & ENC28J60_ADDR_MASK));
   HAL_SPI_Transmit(&hspi3, &data1, 1, 1);
   data1 = ECON2_PKTDEC;
   HAL_SPI_Transmit(&hspi3, &data1, 1, 1);
   delay_us(1);
   this->onClrSelect();
}

// Write register
void CSPI::enc28j60_wcr(uint8_t adr, uint8_t arg)
{
	enc28j60_set_bank(adr);
	enc28j60_write_op(ENC28J60_SPI_WCR, adr, arg);
}

// Write register pair
void CSPI::enc28j60_wcr16(uint8_t adr, uint16_t arg)
{
	enc28j60_set_bank(adr);
	enc28j60_write_op(ENC28J60_SPI_WCR, adr, arg);
	enc28j60_write_op(ENC28J60_SPI_WCR, adr+1, arg>>8);
}

// Clear bits in register (reg &= ~mask)
void CSPI::enc28j60_bfc(uint8_t adr, uint8_t mask)
{
	enc28j60_set_bank(adr);
	enc28j60_write_op(ENC28J60_SPI_BFC, adr, mask);
}

// Set bits in register (reg |= mask)
void CSPI::enc28j60_bfs(uint8_t adr, uint8_t mask)
{
	enc28j60_set_bank(adr);
	enc28j60_write_op(ENC28J60_SPI_BFS, adr, mask);
}

// Read Rx/Tx buffer (at ERDPT)
void CSPI::enc28j60_read_buffer(uint8_t *buf, uint16_t len)
{
   uint8_t data = ENC28J60_SPI_RBM;
   HAL_SPI_Transmit(&hspi3, &data, 1, 1);
	HAL_SPI_Receive(&hspi3, buf, len, 1);                      /** RX SPI PACK - RX SPI PACK - RX SPI PACK - RX SPI PACK **/
	//HAL_SPI_Receive_DMA(&hspi3, buf, len);                   /** RX SPI PACK - RX SPI PACK - RX SPI PACK - RX SPI PACK **/
}

// Write Rx/Tx buffer (at EWRPT)
void CSPI::enc28j60_write_buffer(uint8_t *buf, uint16_t len)
{

   uint8_t data = ENC28J60_SPI_WBM;
   this->onSetSelect();
   HAL_SPI_Transmit(&hspi3, &data, 1, 1);
   HAL_SPI_Transmit_DMA(&hspi3, buf, len);                  /** TX SPI PACK - TX SPI PACK - TX SPI PACK - TX SPI PACK **/
   onStartTimer4(6);                                        /** (5*0.1msec = 0.5msec) -> onSetSelENC28J60(false);   */
	//HAL_SPI_Transmit(&hspi3, buf, len, 1);                       /** TX SPI PACK - TX SPI PACK - TX SPI PACK - TX SPI PACK **/
   //HAL_Delay(1);
   delay_us(10000);
}

// Read PHY register
uint16_t CSPI::enc28j60_read_phy(uint8_t adr)
{
	enc28j60_wcr(MIREGADR, adr);
	enc28j60_bfs(MICMD, MICMD_MIIRD);
	while(enc28j60_rcr(MISTAT) & MISTAT_BUSY) ;
	enc28j60_bfc(MICMD, MICMD_MIIRD);
	return enc28j60_rcr16(MIRD);
}

// Write PHY register
void CSPI::enc28j60_write_phy(uint8_t adr, uint16_t data)
{
	enc28j60_wcr(MIREGADR, adr);
	enc28j60_wcr16(MIWR, data);
	while(enc28j60_rcr(MISTAT) & MISTAT_BUSY) ;
}

// Initiate software reset

void CSPI::enc28j60_soft_reset()
{
   uint32_t i = 0;
   printf("Soft reset\r\n");
//	this->onSetSelect();
//	//enc28j60_tx(ENC28J60_SPI_SC);
//   delay_us(10000);
//   this->onClrSelect();
//   delay_us(10000);

   //this->onSetSelect();
//   delay_us(20000);         //
//   this->onSetReset();
//   delay_us(50000);
//   this->onClrReset();
//   delay_us(50000);
   //this->onClrSelect();

   while(enc28j60_rcr(ECON1) & ECON1_TXRTS)
	{
      if(++i > 3000) break;
		if(enc28j60_rcr(EIR) & EIR_TXERIF)
		{
			enc28j60_bfs(ECON1, ECON1_TXRST);
         delay_us(100);
			enc28j60_bfc(ECON1, ECON1_TXRST);
		};
	};
   i = 0;

   while(enc28j60_rcr(EPKTCNT) > 0)
   {
      enc28j60_bfs(ECON2, ECON2_PKTDEC);
      delay_us(100);
      if(++i > 3000) break;
   };

   //gNextPacketPtr = ENC28J60_RXSTART;
   // clear interrupt flags
//   this->enc28j60_wcr(EIR, 0x00);
   this->enc28j60_set_bank(ECON1);
   this->enc28j60_bfc(ECON1, ECON1_RXEN);
   delay_us(50000);
   this->enc28j60_bfs(ECON1, ECON1_RXEN);
//   this->enc28j60_bfs(EIE, EIE_INTIE|EIE_PKTIE);
   delay_us(1000);

   this->enc28j60_set_bank(MACON2);
   //this->enc28j60_wcr(MACON2, 0xFF);
   delay_us(100);
   this->enc28j60_wcr(MACON2, 0x00);
   delay_us(100);

   this->enc28j60_bfs(MACON2, MACON2_MARST);
   delay_us(100);
   this->enc28j60_bfc(MACON2, MACON2_MARST);
   delay_us(100);
   this->enc28j60_bfs(MACON2, MACON2_MARXRST);
   delay_us(100);
   this->enc28j60_bfc(MACON2, MACON2_MARXRST);
   delay_us(100);
   this->enc28j60_bfs(MACON2, MACON2_RFUNRST);
   delay_us(100);
   this->enc28j60_bfc(MACON2, MACON2_RFUNRST);
   delay_us(100);

   this->enc28j60_set_bank(PHCON1);
   this->enc28j60_write_phy(PHCON1, PHCON1_PPWRSV);
   delay_us(100);
   this->enc28j60_write_phy(PHCON1, 0x0000);
   delay_us(100);
   this->enc28j60_write_phy(PHCON1, PHCON1_PDPXMD);
   delay_us(100);

   //onUpdate();
   //printf("Ethernet restart\r\n");
   //onRestartInitEthernet();
}
//#define SEND_BUFF_SWITCH
void CSPI::enc28j60_send_packet(uint8_t *data, uint16_t len)
{
   uint8_t prcbyte = 0x0E;
   uint8_t tdata = ENC28J60_SPI_WBM;
   enc28j60_bfc(ECON1, ECON1_TXRTS); // Clear packet send
   enc28j60_wcr16(ETXST, ENC28J60_TXSTART);
	enc28j60_wcr16(ETXND, ENC28J60_TXSTART + len + 6);
	enc28j60_wcr16(EWRPT, ENC28J60_TXSTART);

   enc28j60_bfs(ECON1, ECON1_TXRST);
   enc28j60_bfc(ECON1, ECON1_TXRST);
   enc28j60_bfc(EIR, EIR_TXERIF|EIR_TXIF);

   this->onSetSelect();
   HAL_SPI_Transmit(&hspi3, &tdata, 1, 1);
   HAL_SPI_Transmit(&hspi3, &prcbyte, 1, 1);
   delay_us(1);
   this->onClrSelect();
   delay_us(1);
   this->onSetSelect();
   /** TX SPI PACK - TX SPI PACK - TX SPI PACK - TX SPI PACK      */
   HAL_SPI_Transmit(&hspi3, &tdata, 1, 1);
   HAL_SPI_Transmit_DMA(&hspi3, data, len);
   /** StartTimer4 (6*0.1msec = 0.5msec) -> onSetSelENC28J60(false),
      -> enc28j60_bfs(ECON1, ECON1_TXRTS) for request packet send   */
   onStartTimer4(6);
}

void CSPI::onSetReqTX(void)
{
   enc28j60_bfs(ECON1, ECON1_TXRTS); // Request packet send
}

//#define DEBUG_PRINT_1
//#define DEBUG_PRINT_1N
//static volatile uint16_t nextPacket = 0;
//static volatile uint16_t byteCount = 0;
//static volatile uint16_t status = 0;
#define RX_SWITCH
#define RECEIVE_BUFF_SWITCH
sStatusRX mSTRX;
uint8_t* dPtr = 0;

bool CSPI::enc28j60_recv_packet(uint8_t *buf, uint16_t *buflen, bool &next)
{
   /**
   gNextPacketPtr, ERXRDPT - ptr to dataPackNumber 1,2,3...
   ERDPT - ptr to memory address pos
   */
   static bool unreleasedPacket = false;
   static bool lostPacket = false;
   uint8_t data1 = 0;

   if(false) ///unreleasedPacket) *************************************************
   {
      if(gNextPacketPtr == 0)   //(gNextPacketPtr == ENC28J60_RXSTART)
      {
         enc28j60_wcr(ERXRDPTL, (uint8_t)(ENC28J60_RXEND));
         enc28j60_wcr(ERXRDPTH, (uint8_t)((ENC28J60_RXEND)>>8));
      }
      else
      {
         enc28j60_wcr(ERXRDPTL, (uint8_t)(gNextPacketPtr-1));
         enc28j60_wcr(ERXRDPTH, (uint8_t)((gNextPacketPtr-1)>>8));
      };
      unreleasedPacket = false;
   };

   if(false) ///((enc28j60_rcr(EPKTCNT) > 0) || (lostPacket == true)) *************
   {
      enc28j60_wcr(ERDPTL, (gNextPacketPtr));         // Buffer read pointer L
      enc28j60_wcr(ERDPTH, (gNextPacketPtr>>8));      // Buffer read pointer H

      this->onSetSelect();
      uint8_t data = ENC28J60_SPI_RBM;
      HAL_SPI_Transmit(&hspi3, &data, 1, 1);
      HAL_SPI_Receive(&hspi3, (uint8_t*)&mSTRX, 6, 1);            /** RX SPI PACK - RX SPI PACK - RX SPI PACK - RX SPI PACK **/
      delay_us(1);
      this->onClrSelect();
      gNextPacketPtr = mSTRX.nextPacket;
      if(mSTRX.byteCount > 0)
      {
         if((mSTRX.status & 0x80) != 0)
         {
            this->onSetSelect();
            *buflen = mSTRX.byteCount - 4; //remove the CRC count
            HAL_SPI_Transmit(&hspi3, &data, 1, 1);
            HAL_SPI_Receive(&hspi3, buf, *buflen, 1);             /** RX SPI PACK - RX SPI PACK - RX SPI PACK - RX SPI PACK **/
            delay_us(1);
            this->onClrSelect();
            /********************************************/
            //enc28j60_bfs(ECON2, ECON2_PKTDEC);
            enc28j60_set_bank(ECON2);
            this->onSetSelect();
            data1 = (ENC28J60_SPI_BFS | (ECON2 & ENC28J60_ADDR_MASK));
            HAL_SPI_Transmit(&hspi3, &data1, 1, 1);
            data1 = ECON2_PKTDEC;
            HAL_SPI_Transmit(&hspi3, &data1, 1, 1);
            delay_us(1);
            this->onClrSelect();
            /********************************************/
         };
      };
      unreleasedPacket = true;
      enc28j60_set_bank(EIR);
      uint8_t fres = enc28j60_read_op(ENC28J60_SPI_RCR, EIR);
      if(enc28j60_rcr(EPKTCNT) > 0) next = true;
      else if(fres & 0x40) next = true;                           /** if flag PKTIF: Receive Packet Pending in EIR - 1 **/
      else next = false;
      lostPacket = next;

#ifdef DEBUG_PRINT_1
      printf("gNextPacketPtr: %d\r\n", gNextPacketPtr);
      printf("byteCount: %d\r\n", mSTRX.byteCount);
      printf("status: 0x%04X\r\n", mSTRX.status);
      if(next) printf("next - true\r\n\n");
      else printf("next - false\r\n\n");
#endif
      return true;
   };
   return false;
}


// Callback
void HAL_SPI_RxCpltCallback(SPI_HandleTypeDef *hspi)
{
   if (hspi->Instance == SPI3)
   {
      //onSetIRQ_DMA_SPI3();
      printf("DMA RxCpltCallback\n");
   };
}

// Callback
void HAL_SPI_RxCpltCallback2(SPI_HandleTypeDef *hspi)
{
   if (hspi->Instance == SPI3)
   {
      printf("DMA RxCpltCallback 2\n");
   };
}

void HAL_SPI_TxRxCpltCallback(SPI_HandleTypeDef *hspi)
{
    if (hspi->Instance == SPI3)
    {
       pEthernet->m_pLanA->onDmaComplete();
    };
}

//uint8_t rxDR[1600];
//uint16_t CSPI::enc28j60_recv_packet_DMA(uint8_t *buf, uint16_t *len)
//{
//   uint16_t buflen = 0;
//   len = (uint16_t *)&buflen;
//   uint16_t nextPacket = 0;
//   uint16_t byteCount = 0;
//   uint16_t status = 0;
//   static uint16_t gNextPacketPtr = ENC28J60_RXSTART;
//   //&hspi3.Instance->DR = (uint8_t *)rxDR;
//   if(unreleasedPacket)
//   {
//      if (gNextPacketPtr == 0)
//      {
//         enc28j60_wcr(ERXRDPTL, (uint8_t)(ENC28J60_RXEND));
//         enc28j60_wcr(ERXRDPTH, (uint8_t)(ENC28J60_RXEND>>8));
//      }
//      else
//      {
//         enc28j60_wcr(ERXRDPTL, (uint8_t)(gNextPacketPtr - 1));
//         enc28j60_wcr(ERXRDPTH, (uint8_t)((gNextPacketPtr - 1)>>8));
//      };
//      unreleasedPacket = false;
//   };
//   this->enc28j60_set_bank(1);
//   if(enc28j60_rcr(EPKTCNT) > 0)
//   {
//      enc28j60_wcr(ERDPTL, (gNextPacketPtr));         // Buffer read pointer L
//      enc28j60_wcr(ERDPTH, (gNextPacketPtr>>8));      // Buffer read pointer H
//      enc28j60_read_buffer((uint8_t*)&nextPacket, 2);
//		enc28j60_read_buffer((uint8_t*)&byteCount, 2);
//		enc28j60_read_buffer((uint8_t*)&status, 2);
//      gNextPacketPtr = nextPacket;
//      buflen = byteCount - 4; //remove the CRC count
//      this->onSetSelect();
//      if((status & 0x80) != 0)
//      {
//         HAL_DMA_Start(&hdma_spi3_rx, (uint32_t)&hspi3.Instance->DR, (uint32_t)buf, (uint32_t)buflen);
//         HAL_SPI_Receive_DMA(&hspi3, buf, (uint32_t)buflen);
//         //HAL_DMA_PollForTransfer(&hdma_spi3_rx, HAL_DMA_FULL_TRANSFER, HAL_MAX_DELAY);
//         //enc28j60_read_buffer(buf, buflen);
//         enc28j60_bfs(ECON2, ECON2_PKTDEC);
//         unreleasedPacket = true;
//      }
//      else buflen = 0;
//   }
//   else
//   {
//      buflen = 0;
//   };
//   return buflen;
//
//
//
//   //uint16_t buflen = enc28j60_rcr(EPKTCNT);
//   //uint8_t readCommand = ENC28J60_SPI_RBM;
//   //HAL_SPI_Transmit(&hspi3, &readCommand, 1, 1);
//   //buflen = this->ENC28J60_GetReceivedPacketLength();
////   HAL_DMA_Start(&hdma_spi3_rx, (uint32_t)&hspi3.Instance->DR, (uint32_t)buf, (uint32_t)buflen);
////   HAL_SPI_Receive_DMA(&hspi3, buf, (uint32_t)buflen);
////   HAL_DMA_PollForTransfer(&hdma_spi3_rx, HAL_DMA_FULL_TRANSFER, 1);
////   *len = buflen;
//   this->onClrSelect();
//   return buflen;
//}
//*************************************CSPI**********************************

//bool onCheckRxCount(void)
//{
//   if(enc28j60_rcr(EPKTCNT) > 0) return true;
//   else return false;
//}
