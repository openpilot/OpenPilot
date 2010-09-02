/*
 * flash_dfu.h
 *
 *  Created on: 2010/08/31
 *      Author: Programacao
 */

#ifndef FLASH_DFU_H_
#define FLASH_DFU_H_

uint8_t FLASH_Ini();
uint8_t FLASH_Start(uint32_t size);
uint8_t *FLASH_If_Read(uint32_t SectorAddress, uint32_t DataLength);

#endif /* FLASH_DFU_H_ */
