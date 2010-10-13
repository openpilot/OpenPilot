
#ifndef _crc_H
#define _crc_H

#include "stm32f10x.h"

// ********************************************************************

uint16_t UpdateCRC16(uint16_t crc, uint8_t b);
uint16_t UpdateCRC16Data(uint16_t crc, void *data, uint32_t len);

uint32_t UpdateCRC32(uint32_t crc, uint8_t b);
uint32_t UpdateCRC32Data(uint32_t crc, void *data, uint32_t len);

void CRC_init(void);

// ********************************************************************

#endif
