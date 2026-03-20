/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file    i2c.h
  * @brief   This file contains all the function prototypes for
  *          the i2c.c file
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
#ifndef __I2C_H__
#define __I2C_H__

#ifdef __cplusplus
extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#include "main.h"
#include "stdbool.h"
#include "stdint.h"

extern I2C_HandleTypeDef hi2c1;

void MX_I2C1_Init(void);

#ifdef __cplusplus
}
#endif


#define FLASH_CODE_WORD    0x55CC82AA
#define FLASH_TEXT_LEN     32

#pragma pack(push,1)
struct sAddrFLASH                // sAddressFLASH: 128 Bytes R/W
{
   uint32_t codeWord;            // 0x55CC82AA              4B
   uint32_t devNumb;             // device number           4B
   uint8_t devMAC[6];            // MAC address             6B
   uint8_t flagReq;              // request-response flag   1B
   uint8_t flagFunc;             // function                1B
   uint8_t entrance;             // entrance number         1B
   uint8_t numberID;             // ID number for entrance  1B
   uint32_t serverIP;            // local server static IP  4B
   uint32_t routerIP;            // local server static IP  4B
   uint32_t speedRS485;          // USART port speed        4B
   uint16_t crc;                 // crc16                   2B
   char text[FLASH_TEXT_LEN];    //                         32B
};    // 64 Byte
#pragma pack(pop)

class CFlash
{
public:
   CFlash();
   ~CFlash();
   void onInit(void);
   void onWriteByte(uint16_t regAddr, uint8_t Data);
   uint8_t onReceiveByte(uint16_t regAddr);
   uint8_t *onGetOwnMACA(void);
   uint32_t onGetSrvIPA(void);
   char onCheckWritten(void);
   void onWriteProtect(char flg);
   uint16_t onGetDevNumb(void);
   uint32_t onGetSpeedRS485(void);

protected:
private:
   sAddrFLASH m_AddrFL;
   uint8_t *pData;
   uint8_t m_len;
   char m_flag;
};

#endif /* __I2C_H__ */

