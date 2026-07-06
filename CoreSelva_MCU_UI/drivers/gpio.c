#include "gpio.h"
#include <stdint.h>

#define REG32(address) (*(volatile uint32_t *)(address))

#define RCC_BASE        0x58000000UL
#define GPIOA_BASE      0x48000000UL

#define RCC_AHB2ENR     REG32(RCC_BASE + 0x4CUL)
#define GPIOA_MODER     REG32(GPIOA_BASE + 0x00UL)
#define GPIOA_BSRR      REG32(GPIOA_BASE + 0x18UL)
#define GPIOA_ODR       REG32(GPIOA_BASE + 0x14UL)

#define LED_PIN         0U

void GPIO_Init(void)
{
    RCC_AHB2ENR |= (1UL << 0);

    GPIOA_MODER &= ~(3UL << (LED_PIN * 2U));
    GPIOA_MODER |= (1UL << (LED_PIN * 2U));
    GPIO_LED_Off();
}

void GPIO_LED_On(void)
{
    GPIOA_BSRR = 1UL << LED_PIN;
}

void GPIO_LED_Off(void)
{
    GPIOA_BSRR = 1UL << (LED_PIN + 16U);
}

void GPIO_Toggle_LED(void)
{
    if (GPIOA_ODR & (1UL << LED_PIN)) {
        GPIO_LED_Off();
    } else {
        GPIO_LED_On();
    }
}
