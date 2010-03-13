/**
 ******************************************************************************
 *
 * @file       ymodem.c
 * @author     The OpenPilot Team, http://www.openpilot.org Copyright (C) 2010.
 * @brief      YModem functions
 * @see        The GNU Public License (GPL) Version 3
 *
 *****************************************************************************/
/*
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY
 * or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License
 * for more details.
 *
 * You should have received a copy of the GNU General Public License along
 * with this program; if not, write to the Free Software Foundation, Inc.,
 * 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA
 */

/* Includes */
#include "openpilot_bl.h"

/* Local functions */
static int32_t Receive_Byte(uint8_t *c, uint32_t timeout);
static uint32_t Send_Byte(uint8_t c);
static int32_t Receive_Packet(uint8_t *data, int32_t *length, uint32_t timeout);
static uint32_t Str2Int(uint8_t *inputstr, int32_t *intnum);

/* Global variables */
uint32_t FlashDestination = ApplicationAddress; /* Flash user program offset */
uint8_t tab_1024[1024];

/* Local variables */
static uint8_t file_name[FILE_NAME_LENGTH];
static uint16_t PageSize = PAGE_SIZE;
static uint32_t EraseCounter = 0x0;
static uint32_t NbrOfPage = 0;
static FLASH_Status FLASHStatus = FLASH_COMPLETE;
static uint32_t RamSource;


/**
 * Receive a file using the ymodem protocol
 * \param[in]  buf Address of the first byte
 * \return The size of the file
 */
int32_t Ymodem_Receive(uint8_t *buf)
{
	uint8_t packet_data[PACKET_1K_SIZE + PACKET_OVERHEAD], file_size[FILE_SIZE_LENGTH], *file_ptr, *buf_ptr;
	int32_t i, j, packet_length, session_done, file_done, packets_received, errors, session_begin, size = 0;

	/* Initialise FlashDestination variable */
	FlashDestination = ApplicationAddress;

	for(session_done = 0, errors = 0, session_begin = 0;;) {
		for(packets_received = 0, file_done = 0, buf_ptr = buf;;) {
			switch(Receive_Packet(packet_data, &packet_length,
					NAK_TIMEOUT)) {
				case 0:
					errors = 0;
					switch(packet_length) {
						/* Abort by sender */
						case -1:
							Send_Byte(ACK);
							return 0;
							/* End of transmission */
						case 0:
							Send_Byte(ACK);
							file_done = 1;
							break;
							/* Normal packet */
						default:
							if((packet_data[PACKET_SEQNO_INDEX] & 0xff) != (packets_received & 0xff)) {
								Send_Byte(NAK);
							} else {
								if(packets_received == 0) {/* Filename packet */
									if(packet_data[PACKET_HEADER] != 0) {/* Filename packet has valid data */
										for(i = 0, file_ptr = packet_data + PACKET_HEADER; (*file_ptr != 0) && (i < FILE_NAME_LENGTH);) {
											file_name[i++] = *file_ptr++;
										}
										file_name[i++] = '\0';
										for(i = 0, file_ptr++; (*file_ptr != ' ') & (i < FILE_SIZE_LENGTH);) {
											file_size[i++] = *file_ptr++;
										}
										file_size[i++] = '\0';
										Str2Int(file_size, &size);
										/* Test the size of the image to be sent */
										/* Image size is greater than Flash size */
										if(size > (FLASH_SIZE - 1)) {
											/* End session */
											Send_Byte(CA);
											Send_Byte(CA);
											return -1;
										}
										/* Erase the needed pages where the user application will be loaded */
										/* Define the number of page to be erased */
										NbrOfPage = FLASH_PagesMask(size);
										/* Erase the FLASH pages */
										for(EraseCounter = 0; (EraseCounter < NbrOfPage) && (FLASHStatus == FLASH_COMPLETE); EraseCounter++) {
											FLASHStatus = FLASH_ErasePage(FlashDestination + (PageSize * EraseCounter));
										}
										Send_Byte(ACK);
										Send_Byte(CRC16);
									}
									/* Filename packet is empty, end session */
									else {
										Send_Byte(ACK);
										file_done = 1;
										session_done = 1;
										break;
									}
								}
								/* Data packet */
								else {
									memcpy(buf_ptr, packet_data + PACKET_HEADER, packet_length);
									RamSource = (uint32_t) buf;
									for(j = 0; (j < packet_length) && (FlashDestination < ApplicationAddress + size); j += 4) {
										/* Program the data received into STM32F10x Flash */
										FLASH_ProgramWord(FlashDestination, *(uint32_t*) RamSource);
										if(*(uint32_t*) FlashDestination != *(uint32_t*) RamSource) {
											/* End session */
											Send_Byte(CA);
											Send_Byte(CA);
											return -2;
										}
										FlashDestination += 4;
										RamSource += 4;
									}
									Send_Byte(ACK);
								}
								packets_received++;
								session_begin = 1;
							}
					}
					break;
				case 1:
					Send_Byte(CA);
					Send_Byte(CA);
					return -3;
				default:
					if(session_begin > 0) {
						errors++;
					}
					if(errors > MAX_ERRORS) {
						Send_Byte(CA);
						Send_Byte(CA);
						return 0;
					}
					Send_Byte(CRC16);
					break;
			}
			if(file_done != 0) {
				break;
			}
		}
		if(session_done != 0) {
			break;
		}
	}
	return (int32_t) size;
}

/**
 * Receive byte from sender
 * \param[in] c Character
 * \param[in] timeout Timeout
 * \return 0 Byte received
 * \return -1 Timeout
 */
static int32_t Receive_Byte(uint8_t *c, uint32_t timeout)
{
	while(timeout-- > 0) {
		if(PIOS_COM_ReceiveBufferUsed(OPBL_COM_PORT) > 0) {
			*c = PIOS_COM_ReceiveBuffer(OPBL_COM_PORT);
			return 0;
		}
	}
	return -1;
}

/**
 * Send a byte
 * \param[in] c Character
 * \return 0 Byte sent
 */
static uint32_t Send_Byte(uint8_t c)
{
	PIOS_COM_SendChar(OPBL_COM_PORT, c);
	return 0;
}

/**
 * Receive a packet from sender
 * \param[in] data
 * \param[in] length
 * \param[in] timeout
 *     0 end of transmission
 *    -1 abort by sender
 *    >0 packet length
 * \return 0 normally return
 * \return -1 timeout or packet error
 * \return 1 abort by user
 */
static int32_t Receive_Packet(uint8_t *data, int32_t *length, uint32_t timeout)
{
	uint16_t i, packet_size;
	uint8_t c;
	*length = 0;
	if(Receive_Byte(&c, timeout) != 0) {
		return -1;
	}
	switch(c) {
		case SOH:
			packet_size = PACKET_SIZE;
			break;
		case STX:
			packet_size = PACKET_1K_SIZE;
			break;
		case EOT:
			return 0;
		case CA:
			if((Receive_Byte(&c, timeout) == 0) && (c == CA)) {
				*length = -1;
				return 0;
			} else {
				return -1;
			}
		case ABORT1:
		case ABORT2:
			return 1;
		default:
			return -1;
	}
	*data = c;
	for(i = 1; i < (packet_size + PACKET_OVERHEAD); i++) {
		if(Receive_Byte(data + i, timeout) != 0) {
			return -1;
		}
	}
	if(data[PACKET_SEQNO_INDEX] != ((data[PACKET_SEQNO_COMP_INDEX] ^ 0xff)
			& 0xff)) {
		return -1;
	}
	*length = packet_size;
	return 0;
}

/**
 * Convert a string to an integer
 * \param[in] inputstr The string to be converted
 * \param[in] intnum The intger value
 * \return 1 Correct
 * \return 0 Error
 */
static uint32_t Str2Int(uint8_t *inputstr, int32_t *intnum)
{
	uint32_t i = 0, res = 0;
	uint32_t val = 0;

	if(inputstr[0] == '0' && (inputstr[1] == 'x' || inputstr[1] == 'X')) {
		if(inputstr[2] == '\0') {
			return 0;
		}
		for(i = 2; i < 11; i++) {
			if(inputstr[i] == '\0') {
				*intnum = val;
				/* return 1; */
				res = 1;
				break;
			}
			if(ISVALIDHEX(inputstr[i])) {
				val = (val << 4) + CONVERTHEX(inputstr[i]);
			} else {
				/* return 0, Invalid input */
				res = 0;
				break;
			}
		}
		/* over 8 digit hex --invalid */
		if(i >= 11) {
			res = 0;
		}
	} else /* max 10-digit decimal input */
	{
		for(i = 0; i < 11; i++) {
			if(inputstr[i] == '\0') {
				*intnum = val;
				/* return 1 */
				res = 1;
				break;
			} else if((inputstr[i] == 'k' || inputstr[i] == 'K')
					&& (i > 0)) {
				val = val << 10;
				*intnum = val;
				res = 1;
				break;
			} else if((inputstr[i] == 'm' || inputstr[i] == 'M')
					&& (i > 0)) {
				val = val << 20;
				*intnum = val;
				res = 1;
				break;
			} else if(ISVALIDDEC(inputstr[i])) {
				val = val * 10 + CONVERTDEC(inputstr[i]);
			} else {
				/* return 0, Invalid input */
				res = 0;
				break;
			}
		}
		/* Over 10 digit decimal --invalid */
		if(i >= 11) {
			res = 0;
		}
	}

	return res;
}

