/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file    tim.c
  * @brief   This file provides code for the configuration
  *          of the TIM instances.
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

#include "tim.h"
#include "spi.h"
#include "stdio.h"

TIM_HandleTypeDef htim2;
TIM_HandleTypeDef htim3;
TIM_HandleTypeDef htim4;
extern void tickTIMER(void);
extern CEthernet *pEthernet;

#define MBTX_TIME       800

/* TIM2 init function */
void MX_TIM2_Init(void)
{
  TIM_ClockConfigTypeDef sClockSourceConfig = {0};
  TIM_MasterConfigTypeDef sMasterConfig = {0};

  htim2.Instance = TIM2;
  htim2.Init.Prescaler = 31;
  //htim2.Init.Prescaler = 15;
  htim2.Init.CounterMode = TIM_COUNTERMODE_UP;
  htim2.Init.Period = 2625;
  //htim2.Init.Period = 525;
  htim2.Init.ClockDivision = TIM_CLOCKDIVISION_DIV1;
  //htim2.Init.AutoReloadPreload = TIM_AUTORELOAD_PRELOAD_DISABLE;
  if (HAL_TIM_Base_Init(&htim2) != HAL_OK)
  {
    _Error_Handler((char *)__FILE__, __LINE__);
  }
  sClockSourceConfig.ClockSource = TIM_CLOCKSOURCE_INTERNAL;
  //sClockSourceConfig.ClockPolarity = TIM_CLOCKPOLARITY_NONINVERTED;
  //sClockSourceConfig.ClockPrescaler = TIM_CLOCKPRESCALER_DIV1;
  //sClockSourceConfig.ClockFilter = 0;
  if (HAL_TIM_ConfigClockSource(&htim2, &sClockSourceConfig) != HAL_OK)
  {
    _Error_Handler((char *)__FILE__, __LINE__);
  }
  sMasterConfig.MasterOutputTrigger = TIM_TRGO_RESET;
  sMasterConfig.MasterSlaveMode = TIM_MASTERSLAVEMODE_DISABLE;
  if (HAL_TIMEx_MasterConfigSynchronization(&htim2, &sMasterConfig) != HAL_OK)
  {
    _Error_Handler((char *)__FILE__, __LINE__);
  }

	HAL_TIM_Base_Stop(&htim2);
   HAL_TIM_Base_Start_IT(&htim2);
	HAL_TIM_Base_Start(&htim2);
}

/* TIM3 init function */
void MX_TIM3_Init(void)
{

   TIM_ClockConfigTypeDef sClockSourceConfig = {0};
   TIM_MasterConfigTypeDef sMasterConfig = {0};
   TIM_OC_InitTypeDef sConfigOC = {0};

   /* USER CODE BEGIN TIM3_Init 1 */

   /* USER CODE END TIM3_Init 1 */
   htim3.Instance = TIM3;
   htim3.Init.Prescaler = 83;
   htim3.Init.CounterMode = TIM_COUNTERMODE_UP;
   htim3.Init.Period = MBTX_TIME;
   //htim3.Init.Period = 1200;
   htim3.Init.ClockDivision = TIM_CLOCKDIVISION_DIV1;
   htim3.Init.AutoReloadPreload = TIM_AUTORELOAD_PRELOAD_DISABLE;
   if (HAL_TIM_Base_Init(&htim3) != HAL_OK)
   {
      _Error_Handler((char *)__FILE__, __LINE__);
   }
   //sClockSourceConfig.ClockSource = TIM_CLOCKSOURCE_ETRMODE2;
   sClockSourceConfig.ClockSource = TIM_CLOCKSOURCE_INTERNAL;
   sClockSourceConfig.ClockPolarity = TIM_CLOCKPOLARITY_NONINVERTED;
   sClockSourceConfig.ClockPrescaler = TIM_CLOCKPRESCALER_DIV1;
   sClockSourceConfig.ClockFilter = 0;
   if (HAL_TIM_ConfigClockSource(&htim3, &sClockSourceConfig) != HAL_OK)
   {
      _Error_Handler((char *)__FILE__, __LINE__);
   }
   if (HAL_TIM_OC_Init(&htim3) != HAL_OK)
   {
      _Error_Handler((char *)__FILE__, __LINE__);
   }
   if (HAL_TIM_OnePulse_Init(&htim3, TIM_OPMODE_SINGLE) != HAL_OK)
   {
      _Error_Handler((char *)__FILE__, __LINE__);
   }
   sMasterConfig.MasterOutputTrigger = TIM_TRGO_RESET;
   sMasterConfig.MasterSlaveMode = TIM_MASTERSLAVEMODE_DISABLE;
   if (HAL_TIMEx_MasterConfigSynchronization(&htim3, &sMasterConfig) != HAL_OK)
   {
      _Error_Handler((char *)__FILE__, __LINE__);
   }
   sConfigOC.OCMode = TIM_OCMODE_FORCED_ACTIVE;
   sConfigOC.Pulse = 0;
   sConfigOC.OCPolarity = TIM_OCPOLARITY_HIGH;
   sConfigOC.OCFastMode = TIM_OCFAST_DISABLE;
   if (HAL_TIM_OC_ConfigChannel(&htim3, &sConfigOC, TIM_CHANNEL_1) != HAL_OK)
   {
      _Error_Handler((char *)__FILE__, __LINE__);
   }

   //HAL_TIM_MspPostInit(&htim3);

}

void onStartTimer3(uint32_t time)
{
   HAL_TIM_Base_Stop(&htim3);
   TIM3->ARR = time;
   HAL_TIM_Base_Start_IT(&htim3);
	HAL_TIM_Base_Start(&htim3);
}

/* TIM4 init function */
void MX_TIM4_Init(void)
{
   TIM_ClockConfigTypeDef sClockSourceConfig = {0};
   TIM_MasterConfigTypeDef sMasterConfig = {0};

   htim4.Instance = TIM4;
   htim4.Init.Prescaler = 8399;
   htim4.Init.CounterMode = TIM_COUNTERMODE_UP;
   htim4.Init.Period = 9;
   htim4.Init.ClockDivision = TIM_CLOCKDIVISION_DIV1;
   htim4.Init.AutoReloadPreload = TIM_AUTORELOAD_PRELOAD_DISABLE;
   if (HAL_TIM_Base_Init(&htim4) != HAL_OK)
   {
      _Error_Handler((char *)__FILE__, __LINE__);
   };

   sClockSourceConfig.ClockSource = TIM_CLOCKSOURCE_INTERNAL;
   if (HAL_TIM_ConfigClockSource(&htim4, &sClockSourceConfig) != HAL_OK)
   {
      _Error_Handler((char *)__FILE__, __LINE__);
   };

   if (HAL_TIM_OnePulse_Init(&htim4, TIM_OPMODE_SINGLE) != HAL_OK)
   {
      _Error_Handler((char *)__FILE__, __LINE__);
   };

   sMasterConfig.MasterOutputTrigger = TIM_TRGO_RESET;
   sMasterConfig.MasterSlaveMode = TIM_MASTERSLAVEMODE_DISABLE;
   if (HAL_TIMEx_MasterConfigSynchronization(&htim4, &sMasterConfig) != HAL_OK)
   {
      _Error_Handler((char *)__FILE__, __LINE__);
   };
}

void onStartTimer4(uint32_t time)
{
   /** time 1 = 0.1msec*/
   HAL_TIM_Base_Stop(&htim4);
   if(time > 0) --time;
   TIM4->ARR = time;
   HAL_TIM_Base_Start_IT(&htim4);
	HAL_TIM_Base_Start(&htim4);
}

void HAL_TIM_Base_MspInit(TIM_HandleTypeDef* tim_baseHandle)
{

	//GPIO_InitTypeDef GPIO_InitStruct = {0};
	if(tim_baseHandle->Instance==TIM2)
	{
    /* TIM2 clock enable */
		__HAL_RCC_TIM2_CLK_ENABLE();
    /**TIM2 GPIO Configuration
    PA0-WKUP     ------> TIM2_ETR
    */
		//HAL_NVIC_SetPriority(TIM2_IRQn, 0, 3);
		//HAL_NVIC_EnableIRQ(TIM2_IRQn);
	}
   else if(tim_baseHandle->Instance==TIM3)
   {
      /* TIM3 clock enable */
      __HAL_RCC_TIM3_CLK_ENABLE();
      /**TIM3 GPIO Configuration
      PD2     ------> TIM3_ETR
      */

      /* TIM3 interrupt Init */
      HAL_NVIC_SetPriority(TIM3_IRQn, 3, 2);
      HAL_NVIC_EnableIRQ(TIM3_IRQn);
   }
   else if(tim_baseHandle->Instance==TIM4)
   {
      /* TIM4 clock enable */
      __HAL_RCC_TIM4_CLK_ENABLE();

      /* TIM4 interrupt Init */
      HAL_NVIC_SetPriority(TIM4_IRQn, 3, 3);
      HAL_NVIC_EnableIRQ(TIM4_IRQn);
   };
}

void HAL_TIM_Base_MspDeInit(TIM_HandleTypeDef* tim_baseHandle)
{
	if(tim_baseHandle->Instance==TIM2)
	{
		/* Peripheral clock disable */
		__HAL_RCC_TIM2_CLK_DISABLE();

		/**TIM2 GPIO Configuration
		PA0-WKUP     ------> TIM2_ETR
		*/
		HAL_GPIO_DeInit(GPIOA, GPIO_PIN_0);
	}
   else if(tim_baseHandle->Instance==TIM3)
   {
      /* Peripheral clock disable */
      __HAL_RCC_TIM3_CLK_DISABLE();

      /**TIM3 GPIO Configuration
      PA6     ------> TIM3_CH1
      PD2     ------> TIM3_ETR
      */
      HAL_GPIO_DeInit(GPIOA, GPIO_PIN_6);

      HAL_GPIO_DeInit(GPIOD, GPIO_PIN_2);

      /* TIM3 interrupt Deinit */
      HAL_NVIC_DisableIRQ(TIM3_IRQn);
   }
}

void HAL_TIM_MspPostInit(TIM_HandleTypeDef* timHandle)
{

   GPIO_InitTypeDef GPIO_InitStruct = {0};
   if(timHandle->Instance==TIM3)
   {
      __HAL_RCC_GPIOA_CLK_ENABLE();
      /**TIM3 GPIO Configuration
      PA6     ------> TIM3_CH1
      */
      GPIO_InitStruct.Pin = GPIO_PIN_6;
      GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
      GPIO_InitStruct.Pull = GPIO_NOPULL;
      GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
      GPIO_InitStruct.Alternate = GPIO_AF2_TIM3;
      HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);
   }
}

/** timer tic 1 msec */
void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef *htim)
{
   //if(htim == &htim2) tickTIMER2();
   if(htim == &htim4)
   {
      pEthernet->m_pLanA->onClrSelect();
      pEthernet->m_pLanA->onSetReqTX();
   };
}

void HAL_SYSTICK_Callback(void)
{
   tickTIMER();
}

//========= classes ============

CTimeOut::CTimeOut() :
   m_cntT(0),
   m_maxT(1000),
   m_ID(0),
   m_flag(false),
   m_timeOutFlag(false)
{

}

CTimeOut::~CTimeOut()
{

}

void CTimeOut::onTick(void)
{
   if(m_flag)
   {
      if(++m_cntT >= m_maxT)
      {
         m_timeOutFlag = true;
         m_flag = false;
         m_ID = 0;
      };
   };
}

void CTimeOut::onStart(uint32_t tim, uint32_t ID)
{
   //SWO_PrintString("TIMEOUT START\n");
   m_timeOutFlag = false;
   m_maxT = tim;
   m_cntT = 0;
   m_ID = ID;
   m_flag = true;
}

void CTimeOut::onStop(void)
{
   m_flag = false;
   m_timeOutFlag = false;
   //SWO_PrintString("TIMEOUT STOP\n");
}

bool CTimeOut::onIsTimeOut(void)
{
   bool res = false;
   if(m_timeOutFlag == true)
   {
      res = true;
   };
   return res;
}

bool CTimeOut::onIsActive(uint32_t ID)
{
   bool res = false;
   if((m_flag == true) && (m_ID == ID))
   {
      res = true;
   };
   return res;
}

bool CTimeOut::onIsTimerOFF(void)
{
   bool res = false;
   if(m_flag == false) res = true;
   return res;
}

