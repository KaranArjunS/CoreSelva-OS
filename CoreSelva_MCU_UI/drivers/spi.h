#ifndef SPI_H
#define SPI_H

#include <stdint.h>

void SPI1_Init(void);
void SPI1_Write(uint8_t data);

void TFT_CS_Low(void);
void TFT_CS_High(void);
void TFT_DC_Command(void);
void TFT_DC_Data(void);
void TFT_Reset_Low(void);
void TFT_Reset_High(void);

#endif
