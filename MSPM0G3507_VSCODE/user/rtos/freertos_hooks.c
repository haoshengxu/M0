/**
 * @file freertos_hooks.c
 * @brief FreeRTOS 必需回调实现
 */

#include "FreeRTOS.h"
#include "task.h"

/**
 * @brief 栈溢出钩子
 * @param xTask 触发异常的任务句柄
 * @param pcTaskName 触发异常的任务名称
 * @return 无
 */
void vApplicationStackOverflowHook(TaskHandle_t xTask, char *pcTaskName)
{
    (void)xTask;
    (void)pcTaskName;

    while (1)
    {
    }
}
