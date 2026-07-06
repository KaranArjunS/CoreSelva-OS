.syntax unified
.cpu cortex-m4
.fpu fpv4-sp-d16
.thumb


.global Reset_Handler
.global Default_Handler
.global SysTick_Handler


/*
 * Vector table
 * Placed at start of FLASH (0x08000000)
 */
.section .isr_vector,"a",%progbits
.type vector_table, %object


vector_table:

    /* Vector 0: Initial Stack Pointer */
    .word _estack

    /* Vector 1: Reset */
    .word Reset_Handler


    /*
     * ARM Cortex-M4 system exceptions
     * Vector numbers 2 - 15
     */

    .word Default_Handler      /* 2  NMI */
    .word Default_Handler      /* 3  HardFault */
    .word Default_Handler      /* 4  MemManage */
    .word Default_Handler      /* 5  BusFault */
    .word Default_Handler      /* 6  UsageFault */

    .word 0                    /* 7  Reserved */
    .word 0                    /* 8  Reserved */
    .word 0                    /* 9  Reserved */
    .word 0                    /* 10 Reserved */

    .word Default_Handler      /* 11 SVCall */
    .word Default_Handler      /* 12 Debug Monitor */

    .word 0                    /* 13 Reserved */

    .word Default_Handler      /* 14 PendSV */

    .word SysTick_Handler      /* 15 SysTick */


    /*
     * STM32WB55 external interrupts
     * Keep default for now
     */
    .rept 64
        .word Default_Handler
    .endr


.size vector_table, .-vector_table



/*
 * Reset Handler
 * First code executed after reset
 */
.section .text.Reset_Handler,"ax",%progbits
.type Reset_Handler,%function
.thumb_func


Reset_Handler:


    /*
     * Copy .data section
     *
     * From FLASH -> RAM
     */

    ldr r0, =_sidata
    ldr r1, =_sdata
    ldr r2, =_edata


copy_data:

    cmp r1, r2

    bcs zero_bss_start

    ldr r3, [r0], #4

    str r3, [r1], #4

    b copy_data



zero_bss_start:

    /*
     * Clear .bss section
     */

    ldr r1, =_sbss

    ldr r2, =_ebss

    movs r3, #0



zero_bss:

    cmp r1, r2

    bcs call_main

    str r3, [r1], #4

    b zero_bss



call_main:

    /*
     * Enter C application
     */

    bl main



hang:

    b hang


.size Reset_Handler, .-Reset_Handler



/*
 * Default interrupt handler
 */
.section .text.Default_Handler,"ax",%progbits
.type Default_Handler,%function
.thumb_func


Default_Handler:

    b Default_Handler


.size Default_Handler, .-Default_Handler