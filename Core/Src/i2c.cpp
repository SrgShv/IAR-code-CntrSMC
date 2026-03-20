/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file    i2c.c
  * @brief   This file provides code for the configuration
  *          of the I2C instances.
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
#include "i2c.h"

#define EEPROM_ADDRESS  0xA0
#define EEPROM_HW_ADDRESS  0xA0
#define TRUE         1
#define FALSE        0

#define EEPROM_MAXPKT         32              //(page size)
#define EEPROM_WRITE          10              //time to wait in ms
#define EEPROM_TIMEOUT        5*EEPROM_WRITE  //timeout while writing
#define EEPROM_SECTIONSIZE	   64

extern void _Error_Handler(char * file, int line);

I2C_HandleTypeDef hi2c1;

/* I2C1 init function */
void MX_I2C1_Init(void)
{
  hi2c1.Instance = I2C1;
  hi2c1.Init.ClockSpeed = 100000;
  hi2c1.Init.DutyCycle = I2C_DUTYCYCLE_2;
  hi2c1.Init.OwnAddress1 = 0;
  hi2c1.Init.AddressingMode = I2C_ADDRESSINGMODE_7BIT;
  hi2c1.Init.DualAddressMode = I2C_DUALADDRESS_DISABLE;
  hi2c1.Init.OwnAddress2 = 0;
  hi2c1.Init.GeneralCallMode = I2C_GENERALCALL_DISABLE;
  hi2c1.Init.NoStretchMode = I2C_NOSTRETCH_DISABLE;
  if (HAL_I2C_Init(&hi2c1) != HAL_OK)
  {
    _Error_Handler((char *)__FILE__, __LINE__);
  }
}

void HAL_I2C_MspInit(I2C_HandleTypeDef* i2cHandle)
{

  GPIO_InitTypeDef GPIO_InitStruct = {0};
  if(i2cHandle->Instance==I2C1)
  {
    //__HAL_RCC_GPIOB_CLK_ENABLE();
    /**I2C1 GPIO Configuration
    PB6     ------> I2C1_SCL
    PB7     ------> I2C1_SDA
    */
    GPIO_InitStruct.Pin = GPIO_PIN_6|GPIO_PIN_7;
    GPIO_InitStruct.Mode = GPIO_MODE_AF_OD;
    GPIO_InitStruct.Pull = GPIO_PULLUP;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_VERY_HIGH;
    GPIO_InitStruct.Alternate = GPIO_AF4_I2C1;
    HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);

    /* I2C1 clock enable */
    __HAL_RCC_I2C1_CLK_ENABLE();
  }
}

void HAL_I2C_MspDeInit(I2C_HandleTypeDef* i2cHandle)
{

  if(i2cHandle->Instance==I2C1)
  {
    /* Peripheral clock disable */
    __HAL_RCC_I2C1_CLK_DISABLE();

    /**I2C1 GPIO Configuration
    PB6     ------> I2C1_SCL
    PB7     ------> I2C1_SDA
    */
    HAL_GPIO_DeInit(GPIOB, GPIO_PIN_6);
    HAL_GPIO_DeInit(GPIOB, GPIO_PIN_7);
  }
}
/// ================= classes =====================

/**************************** CFlash ****************************/
//char txtr[100];
CFlash::CFlash() :
   m_len(0),
   m_flag(0)
{
   pData = (uint8_t *)&(m_AddrFL);
   m_len = sizeof(m_AddrFL);
}

CFlash::~CFlash()
{
}

void CFlash::onInit(void)
{
   MX_I2C1_Init();
   HAL_Delay(20);
   for(uint8_t i=0; i<m_len; i++)
   {
      pData[i] = this->onReceiveByte(i);
   };
   if(this->onCheckWritten())
   {
      m_flag = 1;
   };
}

void CFlash::onWriteProtect(char flg)
{
   if(flg) HAL_GPIO_WritePin(GPIOE, GPIO_PIN_1, GPIO_PIN_SET);
   else HAL_GPIO_WritePin(GPIOE, GPIO_PIN_1, GPIO_PIN_RESET);
}

void CFlash::onWriteByte(uint16_t regAddr, uint8_t Data)
{
   HAL_I2C_Mem_Write(&hi2c1, EEPROM_ADDRESS, regAddr, I2C_MEMADD_SIZE_8BIT, &Data, 1, 5);
   HAL_Delay(EEPROM_WRITE);
}

uint8_t CFlash::onReceiveByte(uint16_t regAddr)
{
    uint8_t tmp = 0;
    HAL_I2C_Mem_Read(&hi2c1, EEPROM_ADDRESS, regAddr, I2C_MEMADD_SIZE_8BIT, &tmp, 1, 5);
    HAL_Delay(EEPROM_WRITE/2);
    return tmp;
}

char CFlash::onCheckWritten(void)
{
   if(m_AddrFL.codeWord == FLASH_CODE_WORD) return 1;
   else return 0;
}

uint16_t CFlash::onGetDevNumb(void)
{
   return m_AddrFL.devNumb;
}

uint32_t CFlash::onGetSrvIPA(void)
{
   return m_AddrFL.serverIP;
}

uint8_t *CFlash::onGetOwnMACA(void)
{
   return (uint8_t *)&(m_AddrFL.devMAC[0]);
}

uint32_t CFlash::onGetSpeedRS485(void)
{
   return m_AddrFL.speedRS485;
}


