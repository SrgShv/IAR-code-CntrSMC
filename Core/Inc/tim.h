/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file    tim.h
  * @brief   This file contains all the function prototypes for
  *          the tim.c file
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

#ifndef __TIM_H__
#define __TIM_H__

#ifdef __cplusplus
extern "C" {
#endif

#include "main.h"

extern TIM_HandleTypeDef htim4;
extern TIM_HandleTypeDef htim3;
extern TIM_HandleTypeDef htim2;

void MX_TIM4_Init(void);
void MX_TIM3_Init(void);
void MX_TIM2_Init(void);
//void tickTIMER2(void);
void onStartTimer3(uint32_t time);
//void onStartTimer4(uint32_t time);
void onSetSelENC28J60(bool flg);
  
   
#ifdef __cplusplus
}
#endif

class CTimeOut
{
public:
   CTimeOut();
   ~CTimeOut();
   void onTick(void);
   void onStart(uint32_t tim, uint32_t ID);
   void onStop(void);
   bool onIsActive(uint32_t ID);
   bool onIsTimeOut(void);
   bool onIsTimerOFF(void);

protected:
private:
   uint32_t m_cntT;
   uint32_t m_maxT;
   uint32_t m_ID;
   bool m_flag;
   bool m_timeOutFlag;

};

#endif /* __TIM_H__ */

