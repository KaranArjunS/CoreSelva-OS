#ifndef ST7735_H
#define ST7735_H

#include <stdint.h>

#define ST7735_BLACK   0x0000U
#define ST7735_BLUE    0x001FU
#define ST7735_RED     0xF800U
#define ST7735_GREEN   0x07E0U
#define ST7735_CYAN    0x07FFU
#define ST7735_MAGENTA 0xF81FU
#define ST7735_YELLOW  0xFFE0U
#define ST7735_WHITE   0xFFFFU

void ST7735_Init(void);
void ST7735_FillScreen(uint16_t color);
void ST7735_FillRect(uint16_t x, uint16_t y, uint16_t width,
                     uint16_t height, uint16_t color);

#endif
