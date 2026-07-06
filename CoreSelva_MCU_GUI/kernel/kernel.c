#include "kernel.h"
#include <stdint.h>


#define MAX_TASKS 5


/*
 * Task Control Block (small version)
 */
typedef struct
{
    TaskFunction function;   // task function address

    uint32_t delay;          // sleeping time

}Task;



/*
 * Task list
 */
static Task tasks[MAX_TASKS];


static uint8_t task_count = 0;


static uint8_t current_task = 0;




void Kernel_Init(void)
{

    task_count = 0;

    current_task = 0;

}




void Kernel_AddTask(TaskFunction function)
{

    if(task_count < MAX_TASKS)
    {

        tasks[task_count].function = function;


        tasks[task_count].delay = 0;


        task_count++;

    }

}




void Kernel_Start(void)
{

    while(1)
    {

        for(current_task = 0;
            current_task < task_count;
            current_task++)
        {


            /*
             * Run only awake tasks
             */
            if(tasks[current_task].delay == 0)
            {

                tasks[current_task].function();

            }


        }

    }

}




void Kernel_Delay(uint32_t delay)
{

    /*
     * Put current running task to sleep
     */
    tasks[current_task].delay = delay;

}




void Kernel_Tick(void)
{

    /*
     * Called every 1ms by SysTick interrupt
     */
    for(uint8_t i=0; i<task_count; i++)
    {

        if(tasks[i].delay > 0)
        {

            tasks[i].delay--;

        }

    }

}