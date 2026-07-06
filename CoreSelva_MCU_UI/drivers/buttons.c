#include "buttons.h"
#include <stdint.h>

#define REG32(address) (*(volatile uint32_t *)(address))

#define RCC_BASE       0x58000000UL
#define GPIOA_BASE     0x48000000UL
#define GPIOB_BASE     0x48000400UL

#define RCC_AHB2ENR    REG32(RCC_BASE + 0x4CUL)

#define GPIOA_MODER    REG32(GPIOA_BASE + 0x00UL)
#define GPIOA_PUPDR    REG32(GPIOA_BASE + 0x0CUL)
#define GPIOA_IDR      REG32(GPIOA_BASE + 0x10UL)

#define GPIOB_MODER    REG32(GPIOB_BASE + 0x00UL)
#define GPIOB_PUPDR    REG32(GPIOB_BASE + 0x0CUL)
#define GPIOB_IDR      REG32(GPIOB_BASE + 0x10UL)

#define BTN_BACK_PIN   3U
#define BTN_RIGHT_PIN  4U
#define BTN_LEFT_PIN   5U
#define BTN_DOWN_PIN   6U
#define BTN_UP_PIN     7U
#define BTN_ENTER_PIN  15U

static void input_pullup(volatile uint32_t *moder, volatile uint32_t *pupdr,
                         uint32_t pin)
{
    *moder &= ~(3UL << (pin * 2U));
    *pupdr = (*pupdr & ~(3UL << (pin * 2U))) | (1UL << (pin * 2U));
}

void Button_Init(void)
{
    RCC_AHB2ENR |= (1UL << 0) | (1UL << 1);

    input_pullup(&GPIOB_MODER, &GPIOB_PUPDR, BTN_BACK_PIN);
    input_pullup(&GPIOB_MODER, &GPIOB_PUPDR, BTN_RIGHT_PIN);
    input_pullup(&GPIOB_MODER, &GPIOB_PUPDR, BTN_LEFT_PIN);
    input_pullup(&GPIOB_MODER, &GPIOB_PUPDR, BTN_DOWN_PIN);
    input_pullup(&GPIOB_MODER, &GPIOB_PUPDR, BTN_UP_PIN);
    input_pullup(&GPIOA_MODER, &GPIOA_PUPDR, BTN_ENTER_PIN);
}

static uint8_t read_b(uint32_t pin)
{
    return (GPIOB_IDR & (1UL << pin)) ? 0U : 1U;
}

static uint8_t read_a(uint32_t pin)
{
    return (GPIOA_IDR & (1UL << pin)) ? 0U : 1U;
}

uint8_t Button_Up(void)    { return read_b(BTN_UP_PIN); }
uint8_t Button_Down(void)  { return read_b(BTN_DOWN_PIN); }
uint8_t Button_Left(void)  { return read_b(BTN_LEFT_PIN); }
uint8_t Button_Right(void) { return read_b(BTN_RIGHT_PIN); }
uint8_t Button_Back(void)  { return read_b(BTN_BACK_PIN); }
uint8_t Button_Enter(void) { return read_a(BTN_ENTER_PIN); }

uint8_t Button1_Read(void) { return Button_Right(); }
uint8_t Button2_Read(void) { return Button_Left(); }
