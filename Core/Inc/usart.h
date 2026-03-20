/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file    usart.h
  * @brief   This file contains all the function prototypes for
  *          the usart.c file
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
/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __USART_H__
#define __USART_H__

#define USART_RX_BUFFER_SIZE     16

#ifdef __cplusplus
extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#include "main.h"
#include <stdbool.h>
#include <stdint.h>

extern UART_HandleTypeDef huart2;

void MX_USART2_UART_Init(void);
uint16_t onCRC16(const uint8_t *nData, uint16_t wLength);
void onSetRxFlgUSART(bool flg);
uint32_t onGetSpeedRS485(void);
void onShowREG32(uint32_t reg);

#ifdef __cplusplus
}
#endif

class CPortM
{
public:
   CPortM();
   ~CPortM();

   void onInit(void);
   void onClearFlgRX(void);
   void onDeInit(void);
   void onSend(uint8_t *data, uint16_t len);
   void onSetRX(uint16_t RxPackLen);
   uint16_t onGetRxLen(void);
protected:
   uint8_t *m_buffTX;
   uint8_t *m_buffRX;
private:
//   uint8_t *m_CrcTX;
//   uint8_t *m_CrcRX;
   uint16_t m_RxPackLen;
};

class CByteBuff
{
public:
   explicit CByteBuff(uint16_t len);
   ~CByteBuff();

   void onGetData(uint8_t *data, uint16_t len);
   bool onCheck(void);
   void onCopyRX(uint8_t *data, uint16_t &len);

protected:

private:
   uint16_t m_len;
   const uint16_t m_maxSz;
   uint8_t *m_buffRX;
};



#endif /* __USART_H__ */

