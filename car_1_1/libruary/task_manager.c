#include "task_manager.h"

#include <stdbool.h>
#include <stdint.h>

#include "key.h"
#include "oled.h"
#include "../Task/task1.h"

#define TASK_COUNT (4U)

typedef enum {
    TASK_MANAGER_MENU = 0,
    TASK_MANAGER_TASK1,
    TASK_MANAGER_PLACEHOLDER,
    TASK_MANAGER_FAULT
} TaskManager_State;

static TaskManager_State taskState;
static uint8_t selectedTask;

static void TaskManager_Refresh(void)
{
    if (OLED_Refresh() != OLED_STATUS_OK) {
        (void) OLED_Init();
    }
}

static void TaskManager_ShowMenu(void)
{
    uint8_t task;

    OLED_Clear();
    OLED_SetCursor(0U, 0U);
    OLED_WriteString("SELECT TASK");
    for (task = 1U; task <= TASK_COUNT; task++) {
        OLED_SetCursor(0U, (uint8_t) (task * 12U + 4U));
        OLED_WriteString((task == selectedTask) ? ">T" : " T");
        OLED_WriteUInt(task);
    }
    TaskManager_Refresh();
}

static void TaskManager_ShowPlaceholder(void)
{
    OLED_Clear();
    OLED_SetCursor(0U, 16U);
    OLED_WriteString("TASK ");
    OLED_WriteUInt(selectedTask);
    OLED_SetCursor(0U, 40U);
    OLED_WriteString("NOT READY");
    TaskManager_Refresh();
}

static void TaskManager_ReturnToMenu(void)
{
    Task1_Stop();
    taskState = TASK_MANAGER_MENU;
    TaskManager_ShowMenu();
}

void TaskManager_Init(void)
{
    taskState = TASK_MANAGER_MENU;
    selectedTask = 1U;
    Task1_Stop();
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
                selectedTask = (selectedTask == 1U) ? TASK_COUNT : selectedTask - 1U;
                TaskManager_ShowMenu();
            } else if ((pressed & KEY_SW2) != 0U) {
                selectedTask = (selectedTask == TASK_COUNT) ? 1U : selectedTask + 1U;
                TaskManager_ShowMenu();
            } else if ((pressed & KEY_SW3) != 0U) {
                if (selectedTask == 1U) {
                    Task1_Start();
                    taskState = Task1_HasFault() ? TASK_MANAGER_FAULT : TASK_MANAGER_TASK1;
                } else {
                    Task1_Stop();
                    taskState = TASK_MANAGER_PLACEHOLDER;
                    TaskManager_ShowPlaceholder();
                }
            }
            continue;
        }

        if (taskState == TASK_MANAGER_TASK1) {
            Task1_Update();
            if (Task1_HasFault()) {
                taskState = TASK_MANAGER_FAULT;
            }
        }
    }
}
