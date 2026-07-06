#include "spi.h"

#define REG32(address) (*(volatile uint32_t *)(address))

#define RCC_BASE        0x58000000UL
#define GPIOA_BASE      0x48000000UL
#define GPIOB_BASE      0x48000400UL
#define SPI1_BASE       0x40013000UL

#define RCC_AHB2ENR     REG32(RCC_BASE + 0x4CUL)
#define RCC_APB2ENR     REG32(RCC_BASE + 0x60UL)

#define GPIOA_MODER     REG32(GPIOA_BASE + 0x00UL)
#define GPIOA_OSPEEDR   REG32(GPIOA_BASE + 0x08UL)
#define GPIOA_BSRR      REG32(GPIOA_BASE + 0x18UL)
#define GPIOA_AFRL      REG32(GPIOA_BASE + 0x20UL)
#define GPIOB_MODER     REG32(GPIOB_BASE + 0x00UL)
#define GPIOB_OSPEEDR   REG32(GPIOB_BASE + 0x08UL)
#define GPIOB_BSRR      REG32(GPIOB_BASE + 0x18UL)

#define SPI1_CR1        REG32(SPI1_BASE + 0x00UL)
#define SPI1_CR2        REG32(SPI1_BASE + 0x04UL)
#define SPI1_SR         REG32(SPI1_BASE + 0x08UL)
#define SPI1_DR8        (*(volatile uint8_t *)(SPI1_BASE + 0x0CUL))

#define PIN_CS          4U
#define PIN_SCK         5U
#define PIN_MOSI        7U
#define PIN_DC          0U
#define PIN_RESET       1U

static void output_pin(volatile uint32_t *moder, uint32_t pin)
{
    *moder = (*moder & ~(3UL << (pin * 2U))) | (1UL << (pin * 2U));
}

void SPI1_Init(void)
{
    RCC_AHB2ENR |= (1UL << 0) | (1UL << 1); /* GPIOA, GPIOB */
    RCC_APB2ENR |= (1UL << 12);             /* SPI1 */

    /* PA5 and PA7: alternate function 5 (SPI1). */
    GPIOA_MODER = (GPIOA_MODER & ~((3UL << 10) | (3UL << 14))) |
                  (2UL << 10) | (2UL << 14);
    GPIOA_AFRL = (GPIOA_AFRL & ~((0xFUL << 20) | (0xFUL << 28))) |
                 (5UL << 20) | (5UL << 28);
    GPIOA_OSPEEDR |= (3UL << 10) | (3UL << 14);

    output_pin(&GPIOA_MODER, PIN_CS);
    output_pin(&GPIOB_MODER, PIN_DC);
    output_pin(&GPIOB_MODER, PIN_RESET);
    GPIOA_OSPEEDR |= 3UL << (PIN_CS * 2U);
    GPIOB_OSPEEDR |= (3UL << (PIN_DC * 2U)) | (3UL << (PIN_RESET * 2U));

    TFT_CS_High();
    TFT_DC_Data();
    TFT_Reset_High();

    /* Master, mode 0, software NSS, fPCLK/2, one-line transmit only. */
    SPI1_CR1 = (1UL << 2) | (1UL << 8) | (1UL << 9) |
               (1UL << 14) | (1UL << 15);
    /* 8-bit frames and RXNE threshold suitable for 8-bit access. */
    SPI1_CR2 = (7UL << 8) | (1UL << 12);
    SPI1_CR1 |= (1UL << 6);
}

void SPI1_Write(uint8_t data)
{
    while ((SPI1_SR & (1UL << 1)) == 0U) { }
    SPI1_DR8 = data;
    while ((SPI1_SR & (1UL << 7)) != 0U) { }
}

void TFT_CS_Low(void)    { GPIOA_BSRR = 1UL << (PIN_CS + 16U); }
void TFT_CS_High(void)   { GPIOA_BSRR = 1UL << PIN_CS; }
void TFT_DC_Command(void){ GPIOB_BSRR = 1UL << (PIN_DC + 16U); }
void TFT_DC_Data(void)   { GPIOB_BSRR = 1UL << PIN_DC; }
void TFT_Reset_Low(void) { GPIOB_BSRR = 1UL << (PIN_RESET + 16U); }
void TFT_Reset_High(void){ GPIOB_BSRR = 1UL << PIN_RESET; }
