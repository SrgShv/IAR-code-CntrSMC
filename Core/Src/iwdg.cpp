/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file    iwdg.c
  * @brief   This file provides code for the configuration
  *          of the IWDG instances.
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

#include "iwdg.h"

IWDG_HandleTypeDef hiwdg;

/* IWDG init function */
void MX_IWDG_Init(void)
{
   hiwdg.Instance = IWDG;
   hiwdg.Init.Prescaler = IWDG_PRESCALER_32;   // f: 32 kHz, (32000/32) = 1000 Hz, max count 4095
   //hiwdg.Init.Reload = 4095;                  // max count 4095
   hiwdg.Init.Reload = 1000;                    // T = (cnt*Npresc*1)/32000kHz (sec); 1000/1000 Hz -> 1 sec
   if (HAL_IWDG_Init(&hiwdg) != HAL_OK)
   {
      _Error_Handler((char *)__FILE__, __LINE__);
   }
}

void resetWDT(void)
{
   HAL_IWDG_Refresh(&hiwdg);
}

