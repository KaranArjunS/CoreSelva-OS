#include "st7735.h"
#include "spi.h"
#include "systick.h"
#include "tft_config.h"

enum {
    SWRESET = 0x01, SLPOUT = 0x11, NORON = 0x13, INVOFF = 0x20,
    DISPON = 0x29, CASET = 0x2A, RASET = 0x2B, RAMWR = 0x2C,
    MADCTL = 0x36, COLMOD = 0x3A, FRMCTR1 = 0xB1, INVCTR = 0xB4,
    PWCTR1 = 0xC0, PWCTR2 = 0xC1, VMCTR1 = 0xC5
};

static void command(uint8_t value)
{
    TFT_DC_Command();
    TFT_CS_Low();
    SPI1_Write(value);
    TFT_CS_High();
}

static void data(uint8_t value)
{
    TFT_DC_Data();
    TFT_CS_Low();
    SPI1_Write(value);
    TFT_CS_High();
}

static void set_window(uint16_t x0, uint16_t y0, uint16_t x1, uint16_t y1)
{
    x0 += TFT_X_OFFSET; x1 += TFT_X_OFFSET;
    y0 += TFT_Y_OFFSET; y1 += TFT_Y_OFFSET;

    command(CASET);
    data((uint8_t)(x0 >> 8)); data((uint8_t)x0);
    data((uint8_t)(x1 >> 8)); data((uint8_t)x1);
    command(RASET);
    data((uint8_t)(y0 >> 8)); data((uint8_t)y0);
    data((uint8_t)(y1 >> 8)); data((uint8_t)y1);
    command(RAMWR);
}

void ST7735_Init(void)
{
    SPI1_Init();
    TFT_Reset_Low(); delay_ms(20);
    TFT_Reset_High(); delay_ms(150);

    command(SWRESET); delay_ms(150);
    command(SLPOUT);  delay_ms(150);
    command(FRMCTR1); data(0x01); data(0x2C); data(0x2D);
    command(INVCTR);  data(0x07);
    command(PWCTR1);  data(0xA2); data(0x02); data(0x84);
    command(PWCTR2);  data(0xC5);
    command(VMCTR1);  data(0x0E);
    command(INVOFF);
    command(MADCTL);  data(0xC0); /* portrait, RGB order */
    command(COLMOD);  data(0x05); /* RGB565 */
    command(NORON);   delay_ms(10);
    command(DISPON);  delay_ms(100);
}

void ST7735_FillRect(uint16_t x, uint16_t y, uint16_t width,
                     uint16_t height, uint16_t color)
{
    uint32_t pixels;
    if (x >= TFT_WIDTH || y >= TFT_HEIGHT || width == 0U || height == 0U) return;
    if ((uint32_t)x + width > TFT_WIDTH) width = TFT_WIDTH - x;
    if ((uint32_t)y + height > TFT_HEIGHT) height = TFT_HEIGHT - y;

    set_window(x, y, x + width - 1U, y + height - 1U);
    pixels = (uint32_t)width * height;
    TFT_DC_Data();
    TFT_CS_Low();
    while (pixels-- != 0U) {
        SPI1_Write((uint8_t)(color >> 8));
        SPI1_Write((uint8_t)color);
    }
    TFT_CS_High();
}

void ST7735_FillScreen(uint16_t color)
{
    ST7735_FillRect(0U, 0U, TFT_WIDTH, TFT_HEIGHT, color);
}
