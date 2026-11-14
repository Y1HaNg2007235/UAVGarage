/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * File Name          : freertos.c
  * Description        : Code for freertos applications
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2026 STMicroelectronics.
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
#include "FreeRTOS.h"
#include "task.h"
#include "main.h"
#include "FreeRTOS.h"
#include "cmsis_os2.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include "App.h"
/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */

/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */

/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/
/* USER CODE BEGIN Variables */

/* USER CODE END Variables */
/* Definitions for defaultTask */
osThreadId_t defaultTaskHandle;
const osThreadAttr_t defaultTask_attributes = {
  .name = "defaultTask",
  .stack_size = 128 * 4,
  .priority = (osPriority_t) osPriorityNormal,
};
/* Definitions for BattRotate */
osThreadId_t BattRotateHandle;
const osThreadAttr_t BattRotate_attributes = {
  .name = "BattRotate",
  .stack_size = 128 * 4,
  .priority = (osPriority_t) osPriorityHigh,
};
/* Definitions for BattPush */
osThreadId_t BattPushHandle;
const osThreadAttr_t BattPush_attributes = {
  .name = "BattPush",
  .stack_size = 128 * 4,
  .priority = (osPriority_t) osPriorityHigh,
};
/* Definitions for rcProtocolTask */
osThreadId_t rcProtocolTaskHandle;
const osThreadAttr_t rcProtocolTask_attributes = {
  .name = "rcProtocolTask",
  .stack_size = 256 * 4,
  .priority = (osPriority_t) osPriorityNormal,
};
/* Definitions for DUVTransmit */
osThreadId_t DUVTransmitHandle;
const osThreadAttr_t DUVTransmit_attributes = {
  .name = "DUVTransmit",
  .stack_size = 128 * 4,
  .priority = (osPriority_t) osPriorityLow,
};

/* Private function prototypes -----------------------------------------------*/
/* USER CODE BEGIN FunctionPrototypes */

/* USER CODE END FunctionPrototypes */

void StartDefaultTask(void *argument);
extern void StartBattRotate(void *argument);
extern void StartBattPush(void *argument);
void StartrcProtocolTask(void *argument);
void StartDUVTransmit(void *argument);

void MX_FREERTOS_Init(void); /* (MISRA C 2004 rule 8.1) */

/**
  * @brief  FreeRTOS initialization
  * @param  None
  * @retval None
  */
void MX_FREERTOS_Init(void) {
  /* USER CODE BEGIN Init */
  if (!App_Init())
  {
    Error_Handler();
  }
  /* USER CODE END Init */

  /* USER CODE BEGIN RTOS_MUTEX */
  /* add mutexes, ... */
  /* USER CODE END RTOS_MUTEX */

  /* USER CODE BEGIN RTOS_SEMAPHORES */
  /* add semaphores, ... */
  /* USER CODE END RTOS_SEMAPHORES */

  /* USER CODE BEGIN RTOS_TIMERS */
  /* start timers, add new ones, ... */
  /* USER CODE END RTOS_TIMERS */

  /* USER CODE BEGIN RTOS_QUEUES */
  /* add queues, ... */
  /* USER CODE END RTOS_QUEUES */

  /* Create the thread(s) */
  /* creation of defaultTask */
  defaultTaskHandle = osThreadNew(StartDefaultTask, NULL, &defaultTask_attributes);

  /* creation of BattRotate */
  BattRotateHandle = osThreadNew(StartBattRotate, NULL, &BattRotate_attributes);

  /* creation of BattPush */
  BattPushHandle = osThreadNew(StartBattPush, NULL, &BattPush_attributes);

  /* creation of rcProtocolTask */
  rcProtocolTaskHandle = osThreadNew(StartrcProtocolTask, NULL, &rcProtocolTask_attributes);

  /* creation of DUVTransmit */
  DUVTransmitHandle = osThreadNew(StartDUVTransmit, NULL, &DUVTransmit_attributes);

  /* USER CODE BEGIN RTOS_THREADS */
  /* add threads, ... */
  /* USER CODE END RTOS_THREADS */

  /* USER CODE BEGIN RTOS_EVENTS */
  /* add events, ... */
  /* USER CODE END RTOS_EVENTS */

}

/* USER CODE BEGIN Header_StartDefaultTask */
/**
  * @brief  Function implementing the defaultTask thread.
  * @param  argument: Not used
  * @retval None
  */
/* USER CODE END Header_StartDefaultTask */
void StartDefaultTask(void *argument)
{
  /* USER CODE BEGIN StartDefaultTask */
  /* Infinite loop */
  for(;;)
  {
    osDelay(1);
  }
  /* USER CODE END StartDefaultTask */
}

/* USER CODE BEGIN Header_StartBattRotate */
/* StartBattRotate 任务函数体定义在 Application/App.cpp 中，此处仅保留 extern 声明 */
/* USER CODE END Header_StartBattRotate */

/* USER CODE BEGIN Header_StartBattPush */
/* StartBattPush 任务函数体定义在 Application/App.cpp 中，此处仅保留 extern 声明 */
/* USER CODE END Header_StartBattPush */

/* USER CODE BEGIN Header_StartrcProtocolTask */
/**
* @brief Function implementing the rcProtocolTask thread.
* @param argument: Not used
* @retval None
*/
/* USER CODE END Header_StartrcProtocolTask */
void StartrcProtocolTask(void *argument)
{
  /* USER CODE BEGIN StartrcProtocolTask */
  /* Infinite loop */
  for(;;)
  {
    osDelay(1);
  }
  /* USER CODE END StartrcProtocolTask */
}

/* USER CODE BEGIN Header_StartDUVTransmit */
/**
* @brief Function implementing the DUVTransmit thread.
* @param argument: Not used
* @retval None
*/
/* USER CODE END Header_StartDUVTransmit */
void StartDUVTransmit(void *argument)
{
  /* USER CODE BEGIN StartDUVTransmit */
  /* Infinite loop */
  for(;;)
  {
    osDelay(1);
  }
  /* USER CODE END StartDUVTransmit */
}

/* Private application code --------------------------------------------------*/
/* USER CODE BEGIN Application */

/* USER CODE END Application */

