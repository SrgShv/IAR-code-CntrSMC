/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file    gpio.c
  * @brief   This file provides code for the configuration
  *          of all used GPIO pins.
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
#include "gpio.h"
#include "stdio.h"

#define LED_GREEN_Pin            GPIO_PIN_12
#define LED_GREEN_GPIO_Port      GPIOE
#define LED_RED_Pin              GPIO_PIN_13
#define LED_RED_GPIO_Port        GPIOE
#define IRQ_ETH2_Pin             GPIO_PIN_4
#define IRQ_ETH2_GPIO_Port       GPIOC
#define CS_ETH2_Pin              GPIO_PIN_5
#define CS_ETH2_GPIO_Port        GPIOC
#define RESET_ETH2_Pin           GPIO_PIN_0
#define RESET_ETH2_GPIO_Port     GPIOB
#define CS_ETH1_Pin              GPIO_PIN_0
#define CS_ETH1_GPIO_Port        GPIOD
#define RESET_ETH1_Pin           GPIO_PIN_1
#define RESET_ETH1_GPIO_Port     GPIOD
#define IRQ_ETH1_Pin             GPIO_PIN_2
#define IRQ_ETH1_GPIO_Port       GPIOD

/** Configure pins as
        * Analog
        * Input
        * Output
        * EVENT_OUT
        * EXTI
        * Free pins are configured automatically as Analog (this feature is enabled through
        * the Code Generation settings)
*/
void MX_GPIO_Init(void)
{

  GPIO_InitTypeDef GPIO_InitStruct = {0};

  /* GPIO Ports Clock Enable */
  __HAL_RCC_GPIOE_CLK_ENABLE();
  __HAL_RCC_GPIOC_CLK_ENABLE();
  __HAL_RCC_GPIOH_CLK_ENABLE();
  __HAL_RCC_GPIOA_CLK_ENABLE();
  __HAL_RCC_GPIOB_CLK_ENABLE();
  __HAL_RCC_GPIOD_CLK_ENABLE();

//  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(GPIOA, GPIO_PIN_6, GPIO_PIN_RESET);

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(GPIOE, LED_GREEN_Pin|LED_RED_Pin, GPIO_PIN_RESET);

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(GPIOD, CS_ETH1_Pin|RESET_ETH1_Pin|GPIO_PIN_8, GPIO_PIN_RESET);

  /*Configure GPIO pins : LED_GREEN, LED_RED */
  GPIO_InitStruct.Pin = LED_GREEN_Pin|LED_RED_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(GPIOE, &GPIO_InitStruct);

  /*Configure GPIO pins : CS_ETH1, RESET_ETH1 */
  GPIO_InitStruct.Pin = CS_ETH1_Pin|RESET_ETH1_Pin|GPIO_PIN_8;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_VERY_HIGH;
  HAL_GPIO_Init(GPIOD, &GPIO_InitStruct);

  /*Configure GPIO pin : IRQ_ETH1 */
  GPIO_InitStruct.Pin = IRQ_ETH1_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_IT_FALLING;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_VERY_HIGH;
  HAL_GPIO_Init(IRQ_ETH1_GPIO_Port, &GPIO_InitStruct);

    /*Configure GPIO pins : Write protect pin for EEPROM */
  GPIO_InitStruct.Pin = GPIO_PIN_1;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_PULLUP;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(GPIOE, &GPIO_InitStruct);

  /*Configure GPIO pins : PULSE_STROB_RS485 */
  GPIO_InitStruct.Pin = GPIO_PIN_6;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

  HAL_NVIC_SetPriority(EXTI2_IRQn, 0, 1);
  HAL_NVIC_EnableIRQ(EXTI2_IRQn);

}

static volatile bool redL = false;
static volatile bool greenL = false;
void setRedLed(bool flg)
{
   if(flg)
   {
      HAL_GPIO_WritePin(GPIOE, LED_RED_Pin, GPIO_PIN_SET);
      redL = true;
   }
   else
   {
      HAL_GPIO_WritePin(GPIOE, LED_RED_Pin, GPIO_PIN_RESET);
      redL = false;
   };
}

void setBlinkRedLed(bool flg)
{
   if(flg) HAL_GPIO_WritePin(GPIOE, LED_RED_Pin, GPIO_PIN_SET);
   else HAL_GPIO_WritePin(GPIOE, LED_RED_Pin, GPIO_PIN_RESET);
}

bool checkRedL(void)
{
   return redL;
}

bool checkGreenL(void)
{
   return greenL;
}

void setGreenLed(bool flg)
{
   if(flg)
   {
      HAL_GPIO_WritePin(GPIOE, LED_GREEN_Pin, GPIO_PIN_SET);
      greenL = true;
   }
   else
   {
      HAL_GPIO_WritePin(GPIOE, LED_GREEN_Pin, GPIO_PIN_RESET);
      greenL = false;
   };
}

void setBlinkGreenLed(bool flg)
{
   if(flg) HAL_GPIO_WritePin(GPIOE, LED_GREEN_Pin, GPIO_PIN_SET);
   else HAL_GPIO_WritePin(GPIOE, LED_GREEN_Pin, GPIO_PIN_RESET);
}

void setRelayON(bool flg)
{
   if(flg)
   {
      HAL_GPIO_WritePin(GPIOD, GPIO_PIN_8, GPIO_PIN_SET);
   }
   else
   {
      HAL_GPIO_WritePin(GPIOD, GPIO_PIN_8, GPIO_PIN_RESET);
   };
}

void enableExtiIRQ_A(bool flg)
{
   if(flg) HAL_NVIC_EnableIRQ(EXTI2_IRQn);
   else HAL_NVIC_DisableIRQ(EXTI2_IRQn);
}

void setSelLanA(uint8_t flg)
{
   if(flg)
   {
      HAL_GPIO_WritePin(GPIOD, CS_ETH1_Pin, GPIO_PIN_RESET);
   }
   else
   {
      HAL_GPIO_WritePin(GPIOD, CS_ETH1_Pin, GPIO_PIN_SET);
   };
}

void setRstLanA(uint8_t flg)
{
   if(flg) HAL_GPIO_WritePin(GPIOD, RESET_ETH1_Pin, GPIO_PIN_RESET);
   else HAL_GPIO_WritePin(GPIOD, RESET_ETH1_Pin, GPIO_PIN_SET);
}

void setEnableRS485TX(bool flg)
{
   if(flg)
   {
      HAL_GPIO_WritePin(GPIOA, GPIO_PIN_6, GPIO_PIN_SET);
      //printf("1\n");
   }
   else
   {
      HAL_GPIO_WritePin(GPIOA, GPIO_PIN_6, GPIO_PIN_RESET);
      //printf("0\n");
   };
}

