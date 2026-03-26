/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file    stm32f4xx_it.c
  * @brief   Interrupt Service Routines.
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
#include "main.h"
#include "stm32f4xx_it.h"
#include "stdio.h"
/* Private includes ----------------------------------------------------------*/


/* External variables --------------------------------------------------------*/
extern PCD_HandleTypeDef hpcd_USB_OTG_FS;
extern DMA_HandleTypeDef hdma_usart2_rx;
extern DMA_HandleTypeDef hdma_usart2_tx;
extern UART_HandleTypeDef huart2;
extern TIM_HandleTypeDef htim2;
extern TIM_HandleTypeDef htim3;
extern TIM_HandleTypeDef htim4;
extern DMA_HandleTypeDef hdma_spi3_rx;
extern DMA_HandleTypeDef hdma_spi3_tx;
//extern void handleRxIrqLanA(void);
extern void tickTIMER(void);

#define DEBUG_TRACE

/******************************************************************************/
/*           Cortex-M4 Processor Interruption and Exception Handlers          */
/******************************************************************************/
/**
  * @brief This function handles Non maskable interrupt.
  */
void NMI_Handler(void)
{
#ifdef DEBUG_TRACE
assert_failed((uint8_t *)__FILE__, __LINE__);
#endif
   while (1)
  {
  }
  /* USER CODE END NonMaskableInt_IRQn 1 */
}

/**
  * @brief This function handles Hard fault interrupt.
  */

//typedef struct __attribute__((packed)) ContextStateFrame
//{
//  uint32_t r0;
//  uint32_t r1;
//  uint32_t r2;
//  uint32_t r3;
//  uint32_t r12;
//  uint32_t lr;
//  uint32_t return_address;
//  uint32_t xpsr;
//} sContextStateFrame;

void HardFault_Handler(void)
{
#ifdef DEBUG_TRACE
assert_failed((uint8_t *)__FILE__, __LINE__);
#endif

   SystemResetD((char *)__FILE__, __LINE__);
   while (1)
   {
   };
}

/**
  * @brief This function handles Memory management fault.
  */
void MemManage_Handler(void)
{
#ifdef DEBUG_TRACE
assert_failed((uint8_t *)__FILE__, __LINE__);
#endif
  while (1)
  {
    /* USER CODE BEGIN W1_MemoryManagement_IRQn 0 */
    /* USER CODE END W1_MemoryManagement_IRQn 0 */
  }
}

/**
  * @brief This function handles Pre-fetch fault, memory access fault.
  */
void BusFault_Handler(void)
{
#ifdef DEBUG_TRACE
assert_failed((uint8_t *)__FILE__, __LINE__);
#endif
  while (1)
  {
    /* USER CODE BEGIN W1_BusFault_IRQn 0 */
    /* USER CODE END W1_BusFault_IRQn 0 */
  }
}

/**
  * @brief This function handles Undefined instruction or illegal state.
  */
void UsageFault_Handler(void)
{
#ifdef DEBUG_TRACE
assert_failed((uint8_t *)__FILE__, __LINE__);
#endif
  while (1)
  {
    /* USER CODE BEGIN W1_UsageFault_IRQn 0 */
    /* USER CODE END W1_UsageFault_IRQn 0 */
  }
}

/**
  * @brief This function handles System service call via SWI instruction.
  */
void SVC_Handler(void)
{
  /* USER CODE BEGIN SVCall_IRQn 0 */

  /* USER CODE END SVCall_IRQn 0 */
  /* USER CODE BEGIN SVCall_IRQn 1 */

  /* USER CODE END SVCall_IRQn 1 */
}

/**
  * @brief This function handles Debug monitor.
  */
void DebugMon_Handler(void)
{
  /* USER CODE BEGIN DebugMonitor_IRQn 0 */

  /* USER CODE END DebugMonitor_IRQn 0 */
  /* USER CODE BEGIN DebugMonitor_IRQn 1 */

  /* USER CODE END DebugMonitor_IRQn 1 */
}

/**
  * @brief This function handles Pendable request for system service.
  */
void PendSV_Handler(void)
{
  /* USER CODE BEGIN PendSV_IRQn 0 */

  /* USER CODE END PendSV_IRQn 0 */
  /* USER CODE BEGIN PendSV_IRQn 1 */

  /* USER CODE END PendSV_IRQn 1 */
}

/**
  * @brief This function handles System tick timer.
  */
void SysTick_Handler(void)
{
   HAL_IncTick();
   HAL_SYSTICK_IRQHandler();
   //tickTIMER();
}

/******************************************************************************/
/* STM32F4xx Peripheral Interrupt Handlers                                    */
/* Add here the Interrupt Handlers for the used peripherals.                  */
/* For the available peripheral interrupt handler names,                      */
/* please refer to the startup file (startup_stm32f4xx.s).                    */
/******************************************************************************/

/**
  * @brief This function handles DMA1 stream5 global interrupt.
  */
void DMA1_Stream5_IRQHandler(void)
{
  HAL_DMA_IRQHandler(&hdma_usart2_rx);
}

/**
  * @brief This function handles DMA1 stream7 global interrupt.
  */
void DMA1_Stream7_IRQHandler(void)
{
  HAL_DMA_IRQHandler(&hdma_spi3_tx);
}

/**
  * @brief This function handles DMA1 stream0 global interrupt.
  */
void DMA1_Stream0_IRQHandler(void)
{
  HAL_DMA_IRQHandler(&hdma_spi3_rx);
   //printf("DMA RX SPI\r\n");
}

/**
  * @brief This function handles DMA1 stream6 global interrupt.
  */
void DMA1_Stream6_IRQHandler(void)
{
  HAL_DMA_IRQHandler(&hdma_usart2_tx);
}

/**
  * @brief This function handles USART2 global interrupt.
  */
void USART2_IRQHandler(void)
{
  if(__HAL_UART_GET_FLAG(&huart2, UART_FLAG_TC))
  {
    
    //printf("Tx RS485 finished\n\r");
    //setEnableRS485TX(false);
    //onStopTXMB();
    //__HAL_UART_CLEAR_FLAG(&huart2, UART_FLAG_TC);
    // ✔ ФІЗИЧНО все передано
    //DE = 0;
    //RE = 0;

//    __HAL_UART_DISABLE_IT(&huart2, UART_IT_TC);
  };
  HAL_UART_IRQHandler(&huart2);
}

/**
  * @brief This function handles USB On The Go FS global interrupt.
  */
void OTG_FS_IRQHandler(void)
{
  HAL_PCD_IRQHandler(&hpcd_USB_OTG_FS);
}

/**
  * @brief This function handles TIM2 global interrupt.
  */
void TIM2_IRQHandler(void)
{
	HAL_TIM_IRQHandler(&htim2);
	//printf(">, \n");
}

/**
  * @brief This function handles TIM3 global interrupt.
  */
void TIM3_IRQHandler(void)
{
   //printf("t3\n");
   onStopTXMB();
  HAL_TIM_IRQHandler(&htim3);
}

/**
  * @brief This function handles TIM4 global interrupt.
  */
void TIM4_IRQHandler(void)
{
  HAL_TIM_IRQHandler(&htim4);
}

/**
* @brief This function handles EXTI line2 interrupt.
*/
void EXTI2_IRQHandler(void)   /** LAN-A; SPI-3 **/
{
   //handleRxIrqLanA();
   HAL_GPIO_EXTI_IRQHandler(GPIO_PIN_2);
   if(__HAL_GPIO_EXTI_GET_IT(GPIO_PIN_2) != RESET)
   {
      __HAL_GPIO_EXTI_CLEAR_IT(GPIO_PIN_2);
   };
   handleRxIrqLanA();
   //printf("<!>\n");
   //handleRxIrqLanA();
}
