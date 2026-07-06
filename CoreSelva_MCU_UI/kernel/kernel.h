#ifndef KERNEL_H
#define KERNEL_H


#include <stdint.h>


typedef void (*TaskFunction)(void);



void Kernel_Init(void);


void Kernel_AddTask(TaskFunction task);


void Kernel_Start(void);


void Kernel_Delay(uint32_t delay);


void Kernel_Tick(void);



#endif