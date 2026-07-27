#include "task_manager.h"

#include <stdbool.h>
#include <stdint.h>

#include "key.h"
#include "tft.h"
#include "../Task/task1.h"
#include "../Task/task2.h"
#include "../Task/task3.h"
#include "../Task/task4.h"

#define TASK_COUNT (4U)

typedef enum {
    TASK_MANAGER_MENU = 0,
    TASK_MANAGER_TASK1,
    TASK_MANAGER_FAULT
} TaskManager_State;

static TaskManager_State taskState;
static uint8_t selectedTask;



static void TaskManager_ShowMenu(void)
{
    uint8_t task;

    TFT_Clear(TFT_COLOR_BLACK);
    TFT_SetCursor(0U, 0U);
    TFT_WriteString("SELECT TASK");
    for (task = 1U; task <= TASK_COUNT; task++) {
        TFT_SetCursor(0U, (uint8_t) (task * 16U + 8U));
        TFT_WriteString((task == selectedTask) ? ">T" : " T");
        TFT_WriteUInt(task);
    }
    TFT_SetCursor(0U, 88U);
    TFT_WriteString("SW1:NEXT");
    TFT_SetCursor(0U, 104U);
    TFT_WriteString("SW2:RUN");
    TFT_SetCursor(0U, 120U);
    TFT_WriteString("SW4:EXIT");
}

static void TaskManager_ReturnToMenu(void)
{
    Task1_Stop();
    Task2_Stop();
    Task3_Stop();
    Task4_Stop();
    taskState = TASK_MANAGER_MENU;
    TaskManager_ShowMenu();
}

void TaskManager_Init(void)
{
    taskState = TASK_MANAGER_MENU;
    selectedTask = 1U;
    Task1_Stop();
    Task2_Stop();
    Task3_Stop();
    Task4_Stop();
    TaskManager_ShowMenu();
}

void TaskManager_Run(void)
{
    while (true) {
        uint8_t pressed;

        Key_Scan();
        pressed = Key_GetPressed();

        if ((pressed & KEY_SW4) != 0U && taskState != TASK_MANAGER_MENU) {
            TaskManager_ReturnToMenu();
            continue;
        }

        if (taskState == TASK_MANAGER_MENU) {
            if ((pressed & KEY_SW1) != 0U) {
                selectedTask = (selectedTask == TASK_COUNT) ? 1U : selectedTask + 1U;
                TaskManager_ShowMenu();
            } else if ((pressed & KEY_SW2) != 0U) {
                if (selectedTask == 1U) {
                    Task1_Start();
                    taskState = Task1_HasFault() ? TASK_MANAGER_FAULT : TASK_MANAGER_TASK1;
                } else if (selectedTask == 2U) {
                    Task2_Start();
                    taskState = Task2_HasFault() ? TASK_MANAGER_FAULT : TASK_MANAGER_TASK1;
                } else if (selectedTask == 3U) {
                    Task3_Start();
                    taskState = Task3_HasFault() ? TASK_MANAGER_FAULT : TASK_MANAGER_TASK1;
                } else {
                    Task1_Stop();
                    Task2_Stop();
                    Task3_Stop();
                    Task4_Start();
                    taskState = TASK_MANAGER_TASK1;
                }
            }
            continue;
        }

        if (taskState == TASK_MANAGER_TASK1) {
            if (selectedTask == 1U) {
                Task1_Update();
            } else if (selectedTask == 2U) {
                Task2_Update();
            } else if (selectedTask == 3U) {
                Task3_Update();
            } else {
                Task4_Update();
            }
            if ((selectedTask == 1U && Task1_HasFault()) ||
                (selectedTask == 2U && Task2_HasFault()) ||
                (selectedTask == 3U && Task3_HasFault())) {
                taskState = TASK_MANAGER_FAULT;
            }
        }
    }
}
