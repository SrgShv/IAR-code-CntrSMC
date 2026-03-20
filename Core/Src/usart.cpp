/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file    usart.c
  * @brief   This file provides code for the configuration
  *          of the USART instances.
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
/* USER CODE END Header */
/* Includes ------------------------------------------------------------------*/
#include "usart.h"
#include "stdio.h"

#define USART_DMA_BFF_LEN     64
//#define DEBUG_FLASH        /** DEBUG MODE - DEBUG MODE -  DEBUG MODE **/

UART_HandleTypeDef huart2;
DMA_HandleTypeDef hdma_usart2_rx;
DMA_HandleTypeDef hdma_usart2_tx;
extern CByteBuff *pBuffMB;
extern CPortM *pPortMB;
extern void _Error_Handler(char *, int);

/* USART2 init function */

void MX_USART2_UART_Init(void)
{
  huart2.Instance = USART2;

#ifdef DEBUG_FLASH
   huart2.Init.BaudRate = 115200;
#else
   huart2.Init.BaudRate = onGetSpeedRS485();
#endif

  huart2.Init.WordLength = UART_WORDLENGTH_8B;
  huart2.Init.StopBits = UART_STOPBITS_1;
  huart2.Init.Parity = UART_PARITY_NONE;
  huart2.Init.Mode = UART_MODE_TX_RX;
  huart2.Init.HwFlowCtl = UART_HWCONTROL_NONE;
  huart2.Init.OverSampling = UART_OVERSAMPLING_16;
  if (HAL_UART_Init(&huart2) != HAL_OK)
  {
    _Error_Handler((char*)__FILE__, __LINE__);
  }
}

void HAL_UART_MspInit(UART_HandleTypeDef* uartHandle)
{

   GPIO_InitTypeDef GPIO_InitStruct = {0};
   if(uartHandle->Instance==USART2)
   {
      /* DMA controller clock enable */
      __HAL_RCC_DMA1_CLK_ENABLE();

      /* USART2 clock enable */
      __HAL_RCC_USART2_CLK_ENABLE();

      __HAL_RCC_GPIOA_CLK_ENABLE();
      __HAL_RCC_GPIOD_CLK_ENABLE();
      /**USART2 GPIO Configuration
      PA3     ------> USART2_RX
      PD5     ------> USART2_TX
      */

      GPIO_InitStruct.Pin = GPIO_PIN_2|GPIO_PIN_3;
      GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
      GPIO_InitStruct.Pull = GPIO_PULLUP;
      GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_VERY_HIGH;
      GPIO_InitStruct.Alternate = GPIO_AF7_USART2;
      HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

      /* USART2 DMA Init */
      /* USART2_RX Init */
      hdma_usart2_rx.Instance = DMA1_Stream5;
      hdma_usart2_rx.Init.Channel = DMA_CHANNEL_4;
      hdma_usart2_rx.Init.Direction = DMA_PERIPH_TO_MEMORY;
      hdma_usart2_rx.Init.PeriphInc = DMA_PINC_DISABLE;
      hdma_usart2_rx.Init.MemInc = DMA_MINC_ENABLE;
      hdma_usart2_rx.Init.PeriphDataAlignment = DMA_PDATAALIGN_BYTE;
      hdma_usart2_rx.Init.MemDataAlignment = DMA_MDATAALIGN_BYTE;
      hdma_usart2_rx.Init.Mode = DMA_NORMAL;
      hdma_usart2_rx.Init.Priority = DMA_PRIORITY_VERY_HIGH;
      hdma_usart2_rx.Init.FIFOMode = DMA_FIFOMODE_DISABLE;
      if (HAL_DMA_Init(&hdma_usart2_rx) != HAL_OK)
      {
         _Error_Handler((char *)__FILE__, __LINE__);
      }

      __HAL_LINKDMA(uartHandle,hdmarx,hdma_usart2_rx);

      /* USART2_TX Init */
      hdma_usart2_tx.Instance = DMA1_Stream6;
      hdma_usart2_tx.Init.Channel = DMA_CHANNEL_4;
      hdma_usart2_tx.Init.Direction = DMA_MEMORY_TO_PERIPH;
      hdma_usart2_tx.Init.PeriphInc = DMA_PINC_DISABLE;
      hdma_usart2_tx.Init.MemInc = DMA_MINC_ENABLE;
      hdma_usart2_tx.Init.PeriphDataAlignment = DMA_PDATAALIGN_BYTE;
      hdma_usart2_tx.Init.MemDataAlignment = DMA_MDATAALIGN_BYTE;
      hdma_usart2_tx.Init.Mode = DMA_NORMAL;
      //hdma_usart2_tx.Init.Mode = DMA_CIRCULAR;
      hdma_usart2_tx.Init.Priority = DMA_PRIORITY_VERY_HIGH;
      hdma_usart2_tx.Init.FIFOMode = DMA_FIFOMODE_DISABLE;
      if (HAL_DMA_Init(&hdma_usart2_tx) != HAL_OK)
      {
         _Error_Handler((char *)__FILE__, __LINE__);
      }

      __HAL_LINKDMA(uartHandle,hdmatx,hdma_usart2_tx);

      /* USART2 interrupt Init */
      HAL_NVIC_SetPriority(DMA1_Stream5_IRQn, 1, 0);  // RX USART2 DMA
      HAL_NVIC_EnableIRQ(DMA1_Stream5_IRQn);
      HAL_NVIC_SetPriority(DMA1_Stream6_IRQn, 2, 3);  // TX USART2 DMA
      HAL_NVIC_EnableIRQ(DMA1_Stream6_IRQn);

      __HAL_UART_ENABLE_IT(&huart2, UART_IT_ERR);  // Активує переривання для помилок
      __HAL_UART_ENABLE_IT(&huart2, UART_IT_RXNE); // Для отримання даних

      __HAL_DMA_ENABLE_IT(&hdma_usart2_rx, DMA_IT_TE);  // Помилка передачі
      __HAL_DMA_ENABLE_IT(&hdma_usart2_rx, DMA_IT_HT);  // Половинне завершення
      __HAL_DMA_ENABLE_IT(&hdma_usart2_rx, DMA_IT_TC);  // Завершення передачі

      HAL_NVIC_SetPriority(USART2_IRQn, 2, 1);
      HAL_NVIC_EnableIRQ(USART2_IRQn);
   }
}

void HAL_UART_MspDeInit(UART_HandleTypeDef* uartHandle)
{

  if(uartHandle->Instance==USART2)
  {
    /* Peripheral clock disable */
    __HAL_RCC_USART2_CLK_DISABLE();

    /**USART2 GPIO Configuration
    PA3     ------> USART2_RX
    PD5     ------> USART2_TX
    */
    HAL_GPIO_DeInit(GPIOA, GPIO_PIN_3);
    HAL_GPIO_DeInit(GPIOA, GPIO_PIN_2);

    /* USART2 DMA DeInit */
    HAL_DMA_DeInit(uartHandle->hdmarx);
    HAL_DMA_DeInit(uartHandle->hdmatx);

    /* USART2 interrupt Deinit */
    HAL_NVIC_DisableIRQ(USART2_IRQn);
  }
}



static const uint16_t crc16Table [] =
{
    0x0000, 0xC1C0, 0x81C1, 0x4001, 0x01C3, 0xC003, 0x8002, 0x41C2,
    0x01C6, 0xC006, 0x8007, 0x41C7, 0x0005, 0xC1C5, 0x81C4, 0x4004,
    0x01CC, 0xC00C, 0x800D, 0x41CD, 0x000F, 0xC1CF, 0x81CE, 0x400E,
    0x000A, 0xC1CA, 0x81CB, 0x400B, 0x01C9, 0xC009, 0x8008, 0x41C8,
    0x01D8, 0xC018, 0x8019, 0x41D9, 0x001B, 0xC1DB, 0x81DA, 0x401A,
    0x001E, 0xC1DE, 0x81DF, 0x401F, 0x01DD, 0xC01D, 0x801C, 0x41DC,
    0x0014, 0xC1D4, 0x81D5, 0x4015, 0x01D7, 0xC017, 0x8016, 0x41D6,
    0x01D2, 0xC012, 0x8013, 0x41D3, 0x0011, 0xC1D1, 0x81D0, 0x4010,
    0x01F0, 0xC030, 0x8031, 0x41F1, 0x0033, 0xC1F3, 0x81F2, 0x4032,
    0x0036, 0xC1F6, 0x81F7, 0x4037, 0x01F5, 0xC035, 0x8034, 0x41F4,
    0x003C, 0xC1FC, 0x81FD, 0x403D, 0x01FF, 0xC03F, 0x803E, 0x41FE,
    0x01FA, 0xC03A, 0x803B, 0x41FB, 0x0039, 0xC1F9, 0x81F8, 0x4038,
    0x0028, 0xC1E8, 0x81E9, 0x4029, 0x01EB, 0xC02B, 0x802A, 0x41EA,
    0x01EE, 0xC02E, 0x802F, 0x41EF, 0x002D, 0xC1ED, 0x81EC, 0x402C,
    0x01E4, 0xC024, 0x8025, 0x41E5, 0x0027, 0xC1E7, 0x81E6, 0x4026,
    0x0022, 0xC1E2, 0x81E3, 0x4023, 0x01E1, 0xC021, 0x8020, 0x41E0,
    0x01A0, 0xC060, 0x8061, 0x41A1, 0x0063, 0xC1A3, 0x81A2, 0x4062,
    0x0066, 0xC1A6, 0x81A7, 0x4067, 0x01A5, 0xC065, 0x8064, 0x41A4,
    0x006C, 0xC1AC, 0x81AD, 0x406D, 0x01AF, 0xC06F, 0x806E, 0x41AE,
    0x01AA, 0xC06A, 0x806B, 0x41AB, 0x0069, 0xC1A9, 0x81A8, 0x4068,
    0x0078, 0xC1B8, 0x81B9, 0x4079, 0x01BB, 0xC07B, 0x807A, 0x41BA,
    0x01BE, 0xC07E, 0x807F, 0x41BF, 0x007D, 0xC1BD, 0x81BC, 0x407C,
    0x01B4, 0xC074, 0x8075, 0x41B5, 0x0077, 0xC1B7, 0x81B6, 0x4076,
    0x0072, 0xC1B2, 0x81B3, 0x4073, 0x01B1, 0xC071, 0x8070, 0x41B0,
    0x0050, 0xC190, 0x8191, 0x4051, 0x0193, 0xC053, 0x8052, 0x4192,
    0x0196, 0xC056, 0x8057, 0x4197, 0x0055, 0xC195, 0x8194, 0x4054,
    0x019C, 0xC05C, 0x805D, 0x419D, 0x005F, 0xC19F, 0x819E, 0x405E,
    0x005A, 0xC19A, 0x819B, 0x405B, 0x0199, 0xC059, 0x8058, 0x4198,
    0x0188, 0xC048, 0x8049, 0x4189, 0x004B, 0xC18B, 0x818A, 0x404A,
    0x004E, 0xC18E, 0x818F, 0x404F, 0x018D, 0xC04D, 0x804C, 0x418C,
    0x0044, 0xC184, 0x8185, 0x4045, 0x0187, 0xC047, 0x8046, 0x4186,
    0x0182, 0xC042, 0x8043, 0x4183, 0x0041, 0xC181, 0x8180, 0x4040
};

uint16_t onCRC16(const uint8_t *nData, uint16_t len)
{
   uint16_t crc = 0xFFFF;
   for (uint16_t i = 0; i < len; i++)
   {
      crc = (uint16_t)((crc << 8) ^ crc16Table[(crc >> 8) ^ nData[i]]);
   };
   return crc;
}
// ================ classes =================
uint8_t *pBuffRX_MA = 0;
uint8_t *pBuffRX_MB = 0;
uint8_t dma_txb[USART_DMA_BFF_LEN];
uint8_t dma_rxb[USART_DMA_BFF_LEN];

/** ======= main USART COM PORT for RS485 =======*/
CPortM::CPortM() :
   m_buffTX(0),
   m_buffRX(0),
   m_RxPackLen(USART_DMA_BFF_LEN/2)
{
   m_buffTX = dma_txb;
   m_buffRX = dma_rxb;
   pBuffRX_MA = (uint8_t *)&(m_buffRX[0]);
   pBuffRX_MB = (uint8_t *)&(m_buffRX[m_RxPackLen]);
}

CPortM::~CPortM()
{
//    delete [] m_buffTX;
//    delete [] m_buffRX;
}

void CPortM::onInit(void)
{
   huart2.Instance = USART2;

#ifdef DEBUG_FLASH
   huart2.Init.BaudRate = 115200;
#else
   huart2.Init.BaudRate = onGetSpeedRS485();
#endif

   printf("BPS USART:%d\n\r", (int)huart2.Init.BaudRate);
   //huart2.Init.BaudRate = USART_SPEED_BPS;
   huart2.Init.WordLength = UART_WORDLENGTH_8B;
   huart2.Init.StopBits = UART_STOPBITS_1;
   huart2.Init.Parity = UART_PARITY_NONE;
   huart2.Init.Mode = UART_MODE_TX_RX;
   huart2.Init.HwFlowCtl = UART_HWCONTROL_NONE;
   huart2.Init.OverSampling = UART_OVERSAMPLING_16;
   if (HAL_UART_Init(&huart2) != HAL_OK)
   {
      _Error_Handler((char*)__FILE__, __LINE__);
   };
}

//#define USART_REG_PRINT
uint8_t temp = 0;
void CPortM::onClearFlgRX(void)
{
#ifdef USART_REG_PRINT
   uint32_t tempR = 0;
   tempR = huart2.Instance->SR;
   printf("SR : ");
   onShowREG32(tempR);
   tempR = huart2.Instance->CR1;
   printf("CR1: ");
   onShowREG32(tempR);
   tempR = huart2.Instance->CR2;
   printf("CR2: ");
   onShowREG32(tempR);
   tempR = huart2.Instance->CR3;
   printf("CR3: ");
   onShowREG32(tempR);
   printf("------------------\r\n");
#endif

   //uint8_t temp = 0;
   //UNUSED(temp);
   if(__HAL_UART_GET_FLAG(&huart2, UART_FLAG_ORE))
   {
      __HAL_UART_CLEAR_OREFLAG(&huart2);
#ifdef USART_REG_PRINT
      printf("flag ORE\n\r");
#endif
   };
   if(__HAL_UART_GET_FLAG(&huart2, UART_FLAG_FE))
   {
      __HAL_UART_CLEAR_FEFLAG(&huart2);
#ifdef USART_REG_PRINT
      printf("flag FE\n\r");
#endif
   };
   if(__HAL_UART_GET_FLAG(&huart2, UART_FLAG_PE))
   {
      __HAL_UART_CLEAR_PEFLAG(&huart2);
#ifdef USART_REG_PRINT
      printf("flag PE\n\r");
#endif
   };
   if(__HAL_UART_GET_FLAG(&huart2, UART_FLAG_RXNE))
   {
      while(__HAL_UART_GET_FLAG(&huart2, UART_FLAG_RXNE)) temp = huart2.Instance->DR;
#ifdef USART_REG_PRINT
      printf("flag RXNE\n\r");
#endif
   };
   if(__HAL_DMA_GET_FLAG(&hdma_usart2_rx, DMA_FLAG_TCIF0_4))
   {
//      __HAL_DMA_DISABLE(&hdma_usart2_rx);
      __HAL_DMA_CLEAR_FLAG(&hdma_usart2_rx, DMA_FLAG_TCIF0_4);
//      __HAL_DMA_ENABLE(&hdma_usart2_rx);
#ifdef USART_REG_PRINT
      printf("flag DMA TCIF0_4\n\r");
#endif
   };
   if(__HAL_DMA_GET_FLAG(&hdma_usart2_rx, DMA_FLAG_HTIF0_4))
   {
//      __HAL_DMA_DISABLE(&hdma_usart2_rx);
      __HAL_DMA_CLEAR_FLAG(&hdma_usart2_rx, DMA_FLAG_HTIF0_4);
//      __HAL_DMA_ENABLE(&hdma_usart2_rx);
#ifdef USART_REG_PRINT
      printf("flag DMA HTIF0_4\n\r");
#endif
   };
   temp = 0;
}

void CPortM::onDeInit(void)
{
   /* USART2 interrupt Deinit */
   HAL_NVIC_DisableIRQ(USART2_IRQn);
   //HAL_NVIC_DisableIRQ(DMA1_Stream5_IRQn);
   //HAL_NVIC_DisableIRQ(DMA1_Stream6_IRQn);

   /* Peripheral clock disable */
   __HAL_RCC_USART2_CLK_DISABLE();
   HAL_GPIO_DeInit(GPIOA, GPIO_PIN_2|GPIO_PIN_3);

   /* USART2 DMA DeInit */
   /* DMA controller clock enable */
   //__HAL_RCC_DMA1_CLK_DISABLE();
   //HAL_DMA_DeInit(huart2.hdmatx);
   //HAL_DMA_DeInit(huart2.hdmarx);
}

void CPortM::onSend(uint8_t *data, uint16_t len)
{
   for(uint16_t i=0; i<len; i++)
   {
      m_buffTX[i] = data[i];
   };

   setEnableRS485TX(true);

#ifdef DEBUG_FLASH
   uint32_t cnt = computeTxTime(115200, len);
#else
    uint32_t cnt = computeTxTime(onGetSpeedRS485(), len);
#endif

   setEnableTimerTX(cnt);
   huart2.gState = HAL_UART_STATE_READY;
   HAL_UART_Transmit_DMA(&huart2, m_buffTX, len);
}

void CPortM::onSetRX(uint16_t RxPackLen)
{
   m_RxPackLen = RxPackLen;
   if((m_RxPackLen*2) <= USART_DMA_BFF_LEN)
   {
      HAL_UART_Receive_DMA(&huart2, m_buffRX, m_RxPackLen);
   };
}

uint16_t CPortM::onGetRxLen(void)
{
   return m_RxPackLen;
}

//==============================

CByteBuff::CByteBuff(uint16_t len) :
   m_len(0),
   m_maxSz(len),
   m_buffRX(0)
{
   m_buffRX = new uint8_t[m_maxSz];
}

CByteBuff::~CByteBuff()
{
   if(m_buffRX != 0) delete [] m_buffRX;
}

void CByteBuff::onGetData(uint8_t *data, uint16_t len)
{
   if(len <= m_maxSz)
   {
      m_len = len;
      for(uint16_t i=0; i<m_len; i++)
      {
         m_buffRX[i] = data[i];
      };
   };
}

bool CByteBuff::onCheck()
{
   if(m_len > 0) return true;
   else return false;
}

void CByteBuff::onCopyRX(uint8_t *data, uint16_t &len)
{
   len = m_len;
   for(uint16_t i=0; i<m_len; i++)
   {
      data[i] = m_buffRX[i];
   };
   m_len = 0;
}

//==========================

void HAL_UART_RxCpltCallback(UART_HandleTypeDef *huart)
{
   if(huart->Instance == USART2)
   {  /** Modbus RX  */
      onSetRxFlgUSART(true);
      printf("RX:\n");
      //pBuffMB->onGetData(pBuffRX_MA, rxLen);
   };
}

void HAL_UART_RxHalfCpltCallback(UART_HandleTypeDef *huart)
{
   if(huart->Instance == USART2)
   {
   };
}

void HAL_UART_TxCpltCallback(UART_HandleTypeDef *huart)
{
   if(huart->Instance == USART2)
   {
      //printf("txtx\n");
      //HAL_UART_DMAStop(&huart2);
   };
}

void HAL_UART_TxHalfCpltCallback(UART_HandleTypeDef *huart)
{
   if(huart->Instance == USART2)
   {
      //HAL_NVIC_DisableIRQ(DMA1_Stream6_IRQn);
      //HAL_UART_DMAStop(&huart2);
      //HAL_NVIC_EnableIRQ(DMA1_Stream6_IRQn);
      //StartTimeOutMBTX();
      //setEnableCounterTX(false);
      //printf("tx/2\n");
   };
}

/**----------------------------------------------------**/

void HAL_UART_ErrorCallback(UART_HandleTypeDef *huart)
{
   if(huart->Instance == USART2)
   {
      printf("Error USART2 - RS485!!!\r\n");
      pPortMB->onClearFlgRX();
   };
}

void HAL_UART_AbortCpltCallback(UART_HandleTypeDef *huart)
{
   if(huart->Instance == USART2)
   {
      printf("AbortCplt USART2 - RS485!!!\r\n");
      pPortMB->onClearFlgRX();
   };
}

void HAL_UART_AbortTransmitCpltCallback(UART_HandleTypeDef *huart)
{
   if(huart->Instance == USART2)
   {
      printf("AbortTX USART2 - RS485!!!\r\n");
      pPortMB->onClearFlgRX();
   };
}

void HAL_UART_AbortReceiveCpltCallback(UART_HandleTypeDef *huart)
{
   if(huart->Instance == USART2)
   {
      printf("AbortRX USART2 - RS485!!!\r\n");
      pPortMB->onClearFlgRX();
   };
}


