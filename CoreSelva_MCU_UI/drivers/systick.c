#include "systick.h"
#include "kernel.h"
#include <stdint.h>


#define SYSTICK_BASE 0xE000E010UL


#define SYST_CSR \
(*(volatile uint32_t *)(SYSTICK_BASE + 0x00))


#define SYST_RVR \
(*(volatile uint32_t *)(SYSTICK_BASE + 0x04))


#define SYST_CVR \
(*(volatile uint32_t *)(SYSTICK_BASE + 0x08))


volatile uint32_t ticks = 0;


void SysTick_Init(void)
{
    /*
        STM32WB55 default MSI clock
        assume 4 MHz

        4MHz /1000 = 4000

        every 1ms interrupt
    */

    SYST_RVR = 4000 - 1;


    SYST_CVR = 0;


    SYST_CSR =
        (1<<0) |   // enable
        (1<<1) |   // interrupt
        (1<<2);    // CPU clock
}


void SysTick_Handler(void)
{
    ticks++;
    Kernel_Tick();
}



void delay_ms(uint32_t ms)
{
    uint32_t start = ticks;


    while((ticks-start) < ms)
    {

    }
}
