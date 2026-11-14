#include "App.h"
#include "Garage.h"
#include "cmsis_os2.h"
#include "main.h"
#include "tim.h"

// 创建 Garage 类实例，命名为 robot
garage::Garage robot;

/***********************************************************************
** 函 数 名： bool App_Init(void)
** 函数说明： app 初始化
**---------------------------------------------------------------------
** 输入参数： 无
** 返回参数： robot.init()
***********************************************************************/
extern "C" bool App_Init(void)
{
    return robot.init();
}

/***********************************************************************
** 函 数 名： StartBattRotate(void *argument)
** 函数说明： 电池旋转任务，1kHz 执行位置闭环
**---------------------------------------------------------------------
** 输入参数： void *argument
** 返回参数： 无
***********************************************************************/
extern "C" void StartBattRotate(void *argument)
{
    (void)argument;
    for (;;)
    {
        robot.battRotateUpdate();
        osDelay(1);
    }
}

/***********************************************************************
** 函 数 名： StartBattPush(void *argument)
** 函数说明： 电池推送任务，1kHz 执行位置闭环
**---------------------------------------------------------------------
** 输入参数： void *argument
** 返回参数： 无
***********************************************************************/
extern "C" void StartBattPush(void *argument)
{
    (void)argument;
    for (;;)
    {
        robot.battPushUpdate();
        osDelay(1);
    }
}

// 计算任务占用情况的定时器（TIM2，100kHz 计数）
extern "C" void TaskStats_StartTimer(void)
{
    __HAL_TIM_SET_COUNTER(&htim2, 0);
    if (HAL_TIM_Base_Start(&htim2) != HAL_OK)
    {
        Error_Handler();
    }
}

extern "C" uint32_t TaskStats_ReadTimer(void)
{
    return __HAL_TIM_GET_COUNTER(&htim2);
}
