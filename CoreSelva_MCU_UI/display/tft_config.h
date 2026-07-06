#ifndef TFT_CONFIG_H
#define TFT_CONFIG_H

/*
 * STM32WB55 -> 1.8 inch ST7735 (4-wire SPI)
 * PA5 -> SCL/SCK, PA7 -> SDA/MOSI, PA4 -> CS
 * PB0 -> DC/A0,  PB1 -> RESET
 * LED/BLK may be connected directly to 3.3 V, or controlled externally.
 * The display and MCU must share GND. Use 3.3 V logic.
 */
#define TFT_WIDTH       128U
#define TFT_HEIGHT      160U

/* Set these to 2 and 1 for some 128x160 "green tab" panels. */
#define TFT_X_OFFSET    0U
#define TFT_Y_OFFSET    0U

#endif
