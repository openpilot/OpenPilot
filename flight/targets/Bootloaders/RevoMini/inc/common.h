/**
 ******************************************************************************
 * @addtogroup CopterControlBL CopterControl BootLoader
 * @brief These files contain the code to the CopterControl Bootloader.
 *
 * @{
 * @file       common.c
 * @author     The OpenPilot Team, http://www.openpilot.org Copyright (C) 2010.
 * @brief      This file contains various common defines for the BootLoader
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
#ifndef COMMON_H_
#define COMMON_H_

//#include "board.h"

typedef enum {
	start, keepgoing,
} DownloadAction;

/**************************************************/
/* OP_DFU states                       */
/**************************************************/

typedef enum {
	DFUidle, //0
	uploading, //1
	wrong_packet_received, //2
	too_many_packets, //3
	too_few_packets, //4
	Last_operation_Success, //5
	downloading, //6
	BLidle, //7
	Last_operation_failed, //8
	uploadingStarting, //9
	outsideDevCapabilities, //10
	CRC_Fail,//11
	failed_jump,
//12
} DFUStates;
/**************************************************/
/* OP_DFU commands                       */
/**************************************************/
typedef enum {
	Reserved, //0
	Req_Capabilities, //1
	Rep_Capabilities, //2
	EnterDFU, //3
	JumpFW, //4
	Reset, //5
	Abort_Operation, //6
	Upload, //7
	Op_END, //8
	Download_Req, //9
	Download, //10
	Status_Request, //11
	Status_Rep
//12
} DFUCommands;

typedef enum {
	High_Density, Medium_Density
} DeviceType;
/**************************************************/
/* OP_DFU transfer types                       */
/**************************************************/
typedef enum {
	FW, //0
	Descript
//2
} DFUTransfer;
/**************************************************/
/* OP_DFU transfer port                           */
/**************************************************/
typedef enum {
	Usb, //0
	Serial
//2
} DFUPort;
/**************************************************/
/* OP_DFU programable programable HW types        */
/**************************************************/
typedef enum {
	Self_flash, //0
	Remote_flash_via_spi
//1
} DFUProgType;
/**************************************************/
/* OP_DFU programable sources			          */
/**************************************************/
#define USB	0
#define SPI 1

#define DownloadDelay					100000

#define MAX_DEL_RETRYS					3
#define MAX_WRI_RETRYS					3

#endif /* COMMON_H_ */
